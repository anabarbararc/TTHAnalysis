# TTHAnalysis
Threshold analysis alternative code.

Examples are considering board number 01.

## Add files

Create a directory Data_TTH/daq_board_01
``` 
cp Data_TTH/daq_board_01
```
Copy tth_scan data to directory Data_TTH/daq_board_01/tth_scan

## How to run

``` 
source runTTHAnalysis.sh 01
```

## Files created

* Find log files at: Data_TTH/daq_board_01/failed

* Find tth files at: Data_TTH/daq_board_01/tth

* Find root file at: Data_TTH/daq_board_01/freq_vs_tth.root

### Log files

The log files present a list of channels, if any, together with a short explanation why the method might not have worked:
 * frequency (of the chosen tth) is to low or too high
 * frequency is zero for all points or partially
