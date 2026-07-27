from collections import deque
from matplotlib.animation import FuncAnimation
from matplotlib.collections import LineCollection
import matplotlib.pyplot as plt
import serial
import time

SERIAL_PORT = "COM8"

BAUD_RATE = 115200
GRAPH_WINDOW_SECONDS = 15


def main() -> None:
    try:
        esp32 = serial.Serial(
            port=SERIAL_PORT,
            baudrate=BAUD_RATE,
            timeout=0.01,
        )
    except serial.SerialException as error:
        print(f"Could not open {SERIAL_PORT}.")
        print("Close Arduino Serial Monitor and check the COM port.")
        print(f"Error: {error}")
        return

    #Give the ESP32 time to restart after opening the serial connection.
    time.sleep(2)
    esp32.reset_input_buffer()

    sample_times = deque()
    detected_states = deque()

    edge_times = deque()
    edge_directions = deque()

    status = {
        "event_count": 0,
        "latest_state": 0,
    }

    figure, (state_axis, tick_axis) = plt.subplots(
        2,
        1,
        sharex=True,
        figsize=(10, 6),
        height_ratios=[2, 1],
    )

    #Main square-wave signal
    state_line, = state_axis.plot(
        [],
        [],
        drawstyle="steps-post",
        linewidth=2,
        color = "purple"
    )

    #Vertical clock/change ticks
    tick_lines = LineCollection([])
    tick_axis.add_collection(tick_lines)

    tick_points, = tick_axis.plot(
        [],
        [],
        linestyle="none",
        marker="o",
    )

    state_axis.set_ylim(-0.2, 1.2)
    state_axis.set_yticks([0, 1])
    state_axis.set_yticklabels(["No magnet", "Detected"])
    state_axis.set_ylabel("Sensor State")
    state_axis.grid(True)

    tick_axis.set_ylim(-1.3, 1.3)
    tick_axis.set_yticks([-1, 0, 1])
    tick_axis.set_yticklabels(["Removed", "", "Detected"])
    tick_axis.set_ylabel("Change tick")
    tick_axis.set_xlabel("ESP32 time (seconds)")
    tick_axis.grid(True)

    title = state_axis.set_title("Waiting for ESP32 data...")

    def update_graph(_frame):
        #Read every complete serial line currently available
        while esp32.in_waiting > 0:
            try:
                line = (
                    esp32.readline()
                    .decode("utf-8", errors="ignore")
                    .strip()
                )

                parts = line.split(",")

                #Ignore startup messages and malformed lines
                if len(parts) != 3:
                    continue

                time_ms = int(parts[0])
                detected = int(parts[1])
                change = int(parts[2])

                time_seconds = time_ms / 1000.0

                sample_times.append(time_seconds)
                detected_states.append(detected)

                status["latest_state"] = detected

                if change != 0:
                    edge_times.append(time_seconds)
                    edge_directions.append(change)
                    status["event_count"] += 1

                    if change == 1:
                        print(
                            f"{time_seconds:8.3f} s: "
                            f"Magnet detected"
                        )
                    else:
                        print(
                            f"{time_seconds:8.3f} s: "
                            f"Magnet removed"
                        )

            except (ValueError, UnicodeDecodeError):
                #Ignore a partial or damaged serial line
                continue

        if not sample_times:
            return state_line, tick_lines, tick_points, title

        newest_time = sample_times[-1]
        oldest_allowed = newest_time - GRAPH_WINDOW_SECONDS

        #Remove old graph samples
        while sample_times and sample_times[0] < oldest_allowed:
            sample_times.popleft()
            detected_states.popleft()

        #Remove old event markers
        while edge_times and edge_times[0] < oldest_allowed:
            edge_times.popleft()
            edge_directions.popleft()

        state_line.set_data(
            list(sample_times),
            list(detected_states),
        )

        #Construct vertical tick marks from y=0 to y=+1 or y=-1
        tick_segments = [
            [(event_time, 0), (event_time, direction)]
            for event_time, direction in zip(
                edge_times,
                edge_directions,
            )
        ]

        tick_lines.set_segments(tick_segments)
        tick_points.set_data(
            list(edge_times),
            list(edge_directions),
        )

        left_limit = max(0, newest_time - GRAPH_WINDOW_SECONDS)
        right_limit = max(GRAPH_WINDOW_SECONDS, newest_time + 0.1)

        state_axis.set_xlim(left_limit, right_limit)

        if status["latest_state"] == 1:
            state_text = "MAGNET DETECTED"
        else:
            state_text = "NO MAGNET"

        title.set_text(
            f"Current state: {state_text}    |    "
            f"Total changes: {status['event_count']}"
        )

        return state_line, tick_lines, tick_points, title

    animation = FuncAnimation(
        figure,
        update_graph,
        interval=20,
        cache_frame_data=False,
    )

    figure.tight_layout()

    try:
        plt.show()
    finally:
        esp32.close()


if __name__ == "__main__":
    main()