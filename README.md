# TTHAnalysis
Threshold analysis alternative code.

Examples are considering board number 01.

## How to run

``` 
source runTTHAnalysis.sh 01
```

## Files created

* Find log files at: /srv/nfs/rootfs/root/fpga_app/config/Board01/tth_failed

* Find tth files at: /srv/nfs/rootfs/root/fpga_app/config/Board01/tth

* Find root file at: /srv/nfs/rootfs/root/fpga_app/config/Board01/freq_vs_tth.root

### Log files

The log files present a list of channels, if any, together with a short explanation why the method might not have worked:
 * frequency (of the chosen tth) is to low or too high
 * frequency is zero for all points or partially
