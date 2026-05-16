import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# ============================================
# CONFIG
# ============================================
USB_PORT = 'COM5'   # Arduino USB cable  ← change this
BT_PORT  = 'COM7'   # HC-05 Bluetooth    ← change this
BAUD     = 9600
WINDOW   = 200

FINGERS = ['Thumb', 'Index', 'Middle', 'Ring', 'Pinky']
COLORS  = ['#FF4444', '#FF9900', '#44BB44', '#4488FF', '#CC44CC']

data       = [deque([0]*WINDOW, maxlen=WINDOW) for _ in range(5)]
fsr_d      = deque([0]*WINDOW, maxlen=WINDOW)
last_label = ['idle']

# ============================================
# SERIAL CONNECTIONS
# ============================================
try:
    ser_usb = serial.Serial(USB_PORT, BAUD, timeout=1)
    print(f"Graph data: connected to {USB_PORT}")
except Exception as e:
    print(f"ERROR: Could not open USB port {USB_PORT}: {e}")
    exit()

try:
    ser_bt = serial.Serial(BT_PORT, BAUD, timeout=1)
    print(f"Bluetooth:  connected to {BT_PORT}")
    bt_available = True
except Exception as e:
    print(f"WARNING: Bluetooth port {BT_PORT} not open: {e}")
    print("Graph will still work. Commands disabled.")
    bt_available = False

# ============================================
# FIGURE
# ============================================
fig, axes = plt.subplots(6, 1, figsize=(12, 9))
fig.patch.set_facecolor('#1A1A2E')
fig.suptitle('Prosthetic Hand — Live Finger Angles',
             color='white', fontsize=14, fontweight='bold')

lines = []
for i, ax in enumerate(axes[:-1]):
    ax.set_facecolor('#0F0F1A')
    ax.set_ylim(-10, 200)
    ax.set_xlim(0, WINDOW)
    ax.set_ylabel(FINGERS[i], color=COLORS[i], fontsize=9, fontweight='bold')
    ax.tick_params(colors='gray', labelsize=7)
    for spine in ax.spines.values():
        spine.set_color('#333355')
    ax.axhline(y=90, color='#333355', linewidth=0.5, linestyle='--')
    ln, = ax.plot([], [], color=COLORS[i], linewidth=1.2, antialiased=True)
    lines.append(ln)

axes[-1].set_facecolor('#0F0F1A')
axes[-1].set_ylim(-10, 200)
axes[-1].set_xlim(0, WINDOW)
axes[-1].set_ylabel('FSR\nPressure', color='#FFDD44', fontsize=9, fontweight='bold')
axes[-1].tick_params(colors='gray', labelsize=7)
for spine in axes[-1].spines.values():
    spine.set_color('#333355')
fsr_line, = axes[-1].plot([], [], color='#FFDD44', linewidth=1.2)
lines.append(fsr_line)

cmd_text = axes[0].text(
    0.02, 0.85, 'CMD: idle',
    transform=axes[0].transAxes,
    color='white', fontsize=9,
    bbox=dict(boxstyle='round,pad=0.3', facecolor='#333366', alpha=0.8)
)

plt.tight_layout(rect=[0, 0.04, 1, 0.96])

# ============================================
# SEND COMMAND VIA BLUETOOTH
# ============================================
def send_command(cmd: str):
    if bt_available:
        ser_bt.write((cmd.strip() + '\n').encode())
        print(f"Sent: {cmd.strip()}")
    else:
        print("BT not connected. Command not sent.")

# ============================================
# NON-BLOCKING TERMINAL INPUT
# ============================================
import sys, select

def check_stdin():
    if sys.platform == 'win32':
        import msvcrt
        if msvcrt.kbhit():
            line = input("CMD> ")
            send_command(line)
    else:
        r, _, _ = select.select([sys.stdin], [], [], 0)
        if r:
            line = sys.stdin.readline().strip()
            send_command(line)

# ============================================
# READ EMG LINE FROM USB SERIAL
# ============================================
def read_serial():
    try:
        if ser_usb.in_waiting:
            line = ser_usb.readline().decode('utf-8', errors='ignore').strip()
            if line.startswith('EMG,'):
                parts = line.split(',')
                if len(parts) >= 9:
                    angles = [int(parts[j]) for j in range(2, 7)]
                    fsr    = int(parts[7])
                    lbl    = parts[8]
                    return angles, fsr, lbl
    except Exception:
        pass
    return None, None, None

# ============================================
# ANIMATION
# ============================================
color_map = {
    'idle':       '#222244', 'grip':       '#442200',
    'close':      '#440022', 'open':       '#004422',
    'wave_open':  '#003344', 'wave_close': '#003344',
    'peace':      '#224400', 'ok':         '#004444',
    'point':      '#330044', 'thumbsup':   '#443300',
    'reset':      '#222233',
}

def update(frame):
    check_stdin()
    angles, fsr, lbl = read_serial()

    if angles:
        for i in range(5):
            data[i].append(angles[i])
        fsr_d.append(fsr)
        last_label[0] = lbl

    x = list(range(WINDOW))
    for i in range(5):
        lines[i].set_data(x, list(data[i]))
    fsr_line.set_data(x, list(fsr_d))

    lbl = last_label[0]
    cmd_text.set_text(f'CMD: {lbl.upper()}')
    cmd_text.get_bbox_patch().set_facecolor(color_map.get(lbl, '#222244'))

    return lines + [cmd_text]

ani = animation.FuncAnimation(
    fig, update, interval=30, blit=False, cache_frame_data=False
)

print("\n=== Graph running ===")
print("Type commands in this terminal and press Enter to send via Bluetooth.")
print("Examples: open | close | grip | peace | wave | thumbsup | move index 90")
print("Close the graph window to stop.\n")

plt.show()

ser_usb.close()
if bt_available:
    ser_bt.close()
print("Connections closed.")
