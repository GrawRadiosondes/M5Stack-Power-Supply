import time
import lxi
import serial
import numpy as np
import matplotlib.pyplot as plt

# connect to electronic load
info = lxi.LxiInfoClass()


lxi.discover(info, lxi.DiscoverProtocol.VXI11)

load = lxi.Device(info.get_ip('RIGOL TECHNOLOGIES,DL3031A'), 0, "inst0", lxi.Protocol.VXI11)

# connect to M5 Stack Supply
class M5StackSupply:
    def __init__(self, port):
        self.port = serial.Serial(port, 115200, timeout=0.1)
        idn_resp = self.query('*IDN?')
        if not idn_resp.startswith('Graw Radiosondes,M5-PSU 2'):
            raise RuntimeError(f'Wrong instrument at {port}, *IDN? returned {idn_resp}')

    def send_command(self, command: str):
        raw_str = command.encode()
        raw_str += b'\r\n'
        self.port.write(raw_str)

    def query(self, command: str) -> str:
        self.send_command(command)
        line = self.port.readline()
        if len(line) < 2 or line[-2:] != b'\r\n':
            raise RuntimeError('M5StackSupply: invalid response')
        return line[:-2].decode()


supply = M5StackSupply('/dev/ttyACM0')

voltages = [1, 2, 5, 10]
current_max = 2.5
current_step = 0.05

currents = np.arange(0, current_max+1E-4, current_step)

supply.send_command('*RST')
load.send('*RST')

time.sleep(1)

supply.send_command('APPLY 5.0V, 3.0A')
supply.send_command('OUTPUT On')

load.send('Function Current')
load.send('Current 0')
load.send('Input On')

i_supply = np.zeros(currents.size)
i_load =np.zeros(currents.size)

# current measurement at different points
for i in range(currents.size):
    load.send(f'Current {currents[i]}A')

    time.sleep(2)
    i_temp = 0
    for _ in range(10):
        i_temp +=float(supply.query('Measure:Current?'))
        time.sleep(0.2)
    i_supply[i] = i_temp/10
    load.send('Measure:Current?')
    i_load[i] = float(load.receive())
    print(i_supply[i], i_load[i])

# current and voltage measurement noise at one point
supply.send_command('APPLY 5.0V, 3.0A')

load.send(f'Current 0.5A')
load.send('Input On')

time.sleep(2)

num_measures = 100
measure_delay = 0.2

current_results = np.zeros(num_measures)
voltage_results = np.zeros(num_measures)
for i in range(num_measures):
    time.sleep(measure_delay)
    current_results[i] =float(supply.query('Measure:Current?'))
    voltage_results[i] =float(supply.query('Measure:Voltage?'))

current_st_dev = np.std(current_results)
voltage_st_dev = np.std(voltage_results)
print(f'Voltage Standard Deviation: {voltage_st_dev*1e6:.2f} µV')
print(f'Current Standard Deviation: {current_st_dev*1e6:.2f} µA')

load.send('Input Off')

# plot results
offset = (i_supply-i_load)*1000

plt.figure(figsize=(10, 5))

plt.plot(currents,offset)

plt.grid(True)
plt.xlabel('$I [A]$')
plt.ylabel('$\\Delta I [mA]$')

plt.show()
