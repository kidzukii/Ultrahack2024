import matplotlib.pyplot as plt
import pandas as pd

# Load the CSV file into a pandas DataFrame
file_path = 'benchmark.csv'  # Replace with your file path
data = pd.read_csv(file_path)

# Extract the columns for plotting
x = data.iloc[:, 0]  # First column
y = data.iloc[:, 1]  # Second column

# Create the plot
plt.figure(figsize=(10, 6))

# Plot the two columns
plt.plot(x, label='Offset', marker='o', linestyle='-')
plt.plot(y, label='Delay', marker='s', linestyle='--')

# Add labels, title, and legend
plt.xlabel('Time elapsed (s)')  # Replace with an appropriate label
plt.ylabel('Microseconds')  # Replace with an appropriate label
plt.title('Offset and Delay Benchmark')
plt.legend()

# Show the plot
plt.grid()
plt.tight_layout()
plt.show()
