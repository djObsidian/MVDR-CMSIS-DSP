# MVDR/Capon Beamformer
The Minimum Variance Distortionless Response (MVDR) beamformer is a technique used in signal processing, particularly in array signal processing and acoustics, to enhance the reception of a desired signal while suppressing noise and interference from other directions. It is a statistically optimal beamformer. You can read more about it in a simple to understand way at [PySDR website](https://pysdr.org/content/doa.html#mvdr-capon-beamformer)
## Why?
This is part of my personal research on adaptive antenna array for GNSS receiver based on widely available and cheap components. The MVDR algorithm is chosen because it is the simplest of the adaptive beamformers, it is not iterative, and it is agnostic to the shape of the antenna array as well as its inaccuracies. 
## How?
The [CMSIS DSP Library](https://arm-software.github.io/CMSIS-DSP/main/group__groupMatrix.html) has a nice set of matrix functions, which make implementing MVDR very easy! They are built with SIMD and DSP instructions of ARM architecture in mind and are pretty fast as we'll see in benchmark
## Usage
You can see example in this project
```
mvstatus = MVDR_f32(arraySize, snapshotSize, snapshotI, snapshotQ, (float32_t *)qiescentWeights, mvdrWeights);
```
`arraySize` - number of antenna elements in array  
`snapshotSize` - length of snapshot, i.e. number of samples from each antenna to process  
`snapshotI`, `snapshotQ` - samples from antennas arranged in following order:  
`[a1, a2, ... an, b1, b2, ... bn, ...]` where a,b,c,... is array element number  
`qiescentWeights` - initial steering vector  
`mvdrWeights` - output steering vector

To learn more on how it works you can check source code in `MVDR.c`, it's relatively easy to understand

## Perfomance
Tested with snapshot size of 512 and array size of 8 on STM32H723 clocked to 550 MHz with cache disabled. I didn't find any effect of cache on performance here.
| Optimization level  | time to compute, ms |
| ------------- | ------------- |
| -O0  | 16.5 ms  |
| -Ofast  | 13.5 ms  |

CMSIS DSP is already compiled with all possible optimizations, so a small increase is obvious. With a runtime of 13.5 ms, this gives a runtime of about 74 Hz, which is pretty amazing. Assuming a null width of 1 degree, this gives a potential maximum receiver angular velocity of 74 degrees/sec. Not bad for a potential 8 element antenna array for a couple hundred dollars

## To Do
- [ ] Add comparsion to reference weights modeled with Python
- [ ] Add benchmark with 4 element array
- [ ] Test perfomance vs snapshot size
