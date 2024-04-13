import pandas as pd
import matplotlib.pyplot as plt
import json

def plot_data(filename):
    with open(filename, 'r') as file:
        data = [json.loads(line) for line in file]

    df = pd.DataFrame(data)

    df.replace('--', pd.NA, inplace=True)

    # Convert columns to appropriate data types
    df['timestamp'] = pd.to_datetime(df['timestamp'], unit='ms')
    df['out1'] = pd.to_numeric(df['out1'], errors='coerce')
    df['out2'] = pd.to_numeric(df['out2'], errors='coerce')
    df['out3'] = pd.to_numeric(df['out3'], errors='coerce')

    # Plotting
    plt.figure(figsize=(10, 6))
    plt.plot(df['timestamp'], df['out1'], label='Output 1', marker='o')
    plt.plot(df['timestamp'], df['out2'], label='Output 2', marker='o')
    plt.plot(df['timestamp'], df['out3'], label='Output 3', marker='o')
    
    plt.title('Amplitude Values Over Time')
    plt.xlabel('Time')
    plt.ylabel('Amplitude')
    plt.legend()
    plt.grid(True)
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.show()

plot_data('data.txt')