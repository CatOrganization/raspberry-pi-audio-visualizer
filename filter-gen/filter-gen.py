from scipy.signal import butter, freqz, freqs
import matplotlib.pyplot as plt
from math import pi, sin
import numpy as np
import sys

f_s = 44100    # Sample frequency in Hz
f_c = 500     # Cut-off frequency in Hz
order = 4    # Order of the butterworth filter

omega_c = 2 * pi * f_c       # Cut-off angular frequency
omega_c_d = omega_c / f_s    # Normalized cut-off frequency (digital)

# Design the digital Butterworth filter
#b, a = butter(order, omega_c_d / pi)    
b, a = butter(4, 2000, btype='highpass', fs=44100)
print('Coefficients')
print("b =", b)                           # Print the coefficients
print("a =", a)

w, H = freqz(b, a, 4096)                  # Calculate the frequency response
w *= f_s / (2 * pi)                       # Convert from rad/sample to Hz

# Plot the amplitude response
# plt.subplot(2, 1, 1)            
# plt.suptitle('Bode Plot')
# H_dB = 20 * np.log10(abs(H))              # Convert modulus of H to dB
# plt.plot(w, H_dB)
# plt.ylabel('Magnitude [dB]')
# plt.xlim(0, f_s / 2)
# plt.ylim(-80, 6)
# plt.axvline(f_c, color='red')
# plt.axhline(-3, linewidth=0.8, color='black', linestyle=':')

# # Plot the phase response
# plt.subplot(2, 1, 2)
# phi = np.angle(H)                         # Argument of H
# phi = np.unwrap(phi)                      # Remove discontinuities 
# phi *= 180 / pi                           # and convert to degrees
# plt.plot(w, phi)
# plt.xlabel('Frequency [Hz]')
# plt.ylabel('Phase []')
# plt.xlim(0, f_s / 2)
# plt.ylim(-360, 0)
# plt.yticks([-360, -270, -180, -90, 0])
# plt.axvline(f_c, color='red')

### Filter - 6KHz->8Khz Bandpass Filter
### @param [in] input - input unfiltered signal
### @param [out] output - output filtered signal
def filter(x):
    y = [0]*48000
    for n in range(4, len(x)):
        #y[n] = 0.0101*x[n] - 0.0202*x[n-2] + 0.0101*x[n-4] + 2.4354*y[n-1] - 3.1869*y[n-2] + 2.0889*y[n-3] - 0.7368*y[n-4] 
        inSum = 0
        for i in range(len(b)):
        	inSum += b[i] * x[n-i]

        outSum = 0
        for i in range(1, len(a)):
        	outSum += a[i] * y[n-i]
        y[n] = inSum - outSum

        if n == 8:
        	print "inSum: ", inSum, "outSum: ", outSum, "y[n]: ", y[n]
    return y

###Read in desired frequency from command line
frequency = int(sys.argv[1])	
	
### Create empty arrays
input = [0]*480
output = [0]*480

### Fill array with xxxHz signal
for i in range(480):
    input[i] = sin(2 * pi * frequency * i / 44100) #+ sin(2 * pi * 70 * i / 48000)

### Run the signal through the filter
output = filter(input)

### Grab samples from input and output #1/100th of a second
output_section = output[0:480]  
input_section = input[0:480] 

### Plot the signals for comparison
plt.figure(1)                
plt.subplot(211)   
plt.ylabel('Magnitude')
plt.xlabel('Samples') 
plt.title('Unfiltered Signal')      
plt.plot(input_section)
plt.subplot(212)             
plt.ylabel('Magnitude')
plt.xlabel('Samples') 
plt.title('Filtered Signal')
plt.plot(output_section)


plt.show()
