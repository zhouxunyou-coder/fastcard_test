# PK50G ftp加pktgen混合测试

------

## 测试环境
1. **cpu:** Intel(R) Xeon(R) E-2176G CPU @ 3.70GHz(共12核)
2. **meminfo:** 双条8G(共16G)
3. **OS:** Pakflow OS
4. **平台信息:** C246
5. **逻辑文件:** \\192.168.0.130\生产版本\PK50G_4W4F4C\test\tesc.mcs `(日期:2019-09-17)`  
MD5: 9DC8FCF621B5FD207356CB19C9541271
6. **pci及板卡逻辑信息:**

```shell
# lspci -vv | grep -A 33 Xilinx
01:00.0 Memory controller: Xilinx Corporation Device 7024
        Subsystem: Xilinx Corporation Device 0007
        Control: I/O+ Mem+ BusMaster+ SpecCycle- MemWINV- VGASnoop- ParErr- Stepping- SERR- FastB2B- DisINTx-
        Status: Cap+ 66MHz- UDF- FastB2B- ParErr- DEVSEL=fast >TAbort- <TAbort- <MAbort- >SERR- <PERR- INTx-
        Latency: 0, Cache Line Size: 64 bytes
        Region 0: Memory at 90000000 (32-bit, non-prefetchable) [size=256M]
        Capabilities: [40] Power Management version 3
                Flags: PMEClk- DSI- D1- D2- AuxCurrent=0mA PME(D0+,D1+,D2+,D3hot+,D3cold-)
                Status: D0 NoSoftRst+ PME-Enable- DSel=0 DScale=0 PME-
        Capabilities: [60] Express (v2) Endpoint, MSI 00
                DevCap: MaxPayload 512 bytes, PhantFunc 0, Latency L0s <64ns, L1 unlimited
                        ExtTag- AttnBtn- AttnInd- PwrInd- RBE+ FLReset-
                DevCtl: Report errors: Correctable- Non-Fatal- Fatal- Unsupported-
                        RlxdOrd+ ExtTag- PhantFunc- AuxPwr- NoSnoop+
                        MaxPayload 256 bytes, MaxReadReq 512 bytes
                DevSta: CorrErr- UncorrErr- FatalErr- UnsuppReq- AuxPwr- TransPend-
                LnkCap: Port #0, Speed 5GT/s, Width x4, ASPM L0s, Latency L0 unlimited, L1 unlimited
                        ClockPM- Surprise- LLActRep- BwNot-
                LnkCtl: ASPM Disabled; RCB 64 bytes Disabled- Retrain- CommClk+
                        ExtSynch- ClockPM- AutWidDis- BWInt- AutBWInt-
                LnkSta: Speed 5GT/s, Width x4, TrErr- Train- SlotClk+ DLActive- BWMgmt- ABWMgmt-
                DevCap2: Completion Timeout: Range B, TimeoutDis-
                DevCtl2: Completion Timeout: 50us to 50ms, TimeoutDis-
                LnkCtl2: Target Link Speed: 5GT/s, EnterCompliance- SpeedDis-, Selectable De-emphasis: -6dB
                         Transmit Margin: Normal Operating Range, EnterModifiedCompliance- ComplianceSOS-
                         Compliance De-emphasis: -6dB
                LnkSta2: Current De-emphasis Level: -6dB
        Capabilities: [9c] MSI-X: Enable+ Count=32 Masked-
                Vector table: BAR=0 offset=00100000
                PBA: BAR=0 offset=00100100
        Capabilities: [100 v1] Device Serial Number 00-00-00-01-01-00-0a-35
        Kernel driver in use: asic
        
# md 0 40
  0: 187ec000 0024f908 00000000 00000000 00000008 00000000 00378300 013e013f
 20: 0000007a 00000000 00010009 00001140 0000796d 00000000 00040200 00000000
 40: 00000000 00000000 00000000 00000000 00a0f0a5 000000af 00000000 00000000
 60: 02026c6c 00007d71 00000000 00000000 00000000 00000000 00000000 00000000
 80: 00000001 00000001 00000001 00000001 00000001 00000001 00000001 00000001
 a0: 00000000 00000000 00000000 00000000 00000001 00000001 00000001 00000001
 c0: 00000001 00000001 00000001 00000001 00378400 00000000 00000000 00000000
 e0: 00000000 00000000 00000000 00000000 00000000 00000000 00000003 00000000
```
---
## 测试项
### 1. 一对万兆口，路由模式下ftp传文件
#### 1.1 测试环境配置
- 待测机器上执行:
```
brctl delif br104 eth10
brctl delif br104 eth11
ifconfig eth10 7.7.7.227
ifconfig eth11 12.12.12.227
```
- ftp服务器上执行：
```
ifconfig eth12 12.12.12.12
route add default gw 12.12.12.227
```
- ftp客户端上执行：
```
ifconfig eth1 7.7.7.117
route add default gw 7.7.7.227
```
- 在ftp客户端执行 `ping 12.12.12.12`若可以ping通证明环境接线没问题，可以开始传输文件了。
- 传输文件执行:`ftpget -u pakflow -f fedora 12.12.12.12 ./test.img `

#### 1.2 测试结果
- 多次成功传输3.5G大文件，和4K小文件。
- 在传输大文件的时候，通过netdev查看快转的流量在300M左右。
---
### 2. 桥模式下千兆电口、千兆光口、万兆口同时打pktgen，每条pktgen打100万条流。
#### 2.1 测试环境配置
- 配置udp老化时间2秒:`echo 2 > /proc/sys/net/netfilter/nf_conntrack_udp_timeout_stream`

#### 2.2 测试结果 
- 60小时测试后，未出现异常。在现有环境上执行ping测试正常。
- [md 0 40](https://www.qq.com 
"  00: 187ec000 0024f908 00000000 00000000 00000008 00000000 0037c300 02800281
 20: 00001000 00000000 00a10009 00001140 000001c8 00000000 00000000 00000000
 40: 00000000 00000000 00000000 00000000 fa0a05f5 00000000 00000000 00000c0c
 60: 07070707 00007d71 00000000 00000000 00000000 00000000 00000000 00000000
 80: 00000001 00000001 00000001 00000001 00000001 00000001 00000001 00000001
 a0: 00000000 00000000 00000000 00000000 00000001 00000001 00000001 00000001
 c0: 00000001 00000001 00000001 00000001 0037c400 00000000 00000000 00000000
 e0: 00000000 00000000 00000000 00000000 00000000 00000000 0000000c 00000000")信息
---
### 3. 千兆电口与万兆口路由模式下ftp传文件，千兆光口打桥模式pktgen 100万条流。
#### 3.1 测试环境配置
- 路由配置参照测试一
- udp老化时间配置设置2秒，参考测试二

#### 3.2 测试结果
- 在打pktgen的过程中，ftp传文件偶尔不成功。停下pktgen时，现象依然存在。

    - 问题原因：
    > 经排查发现，出现问题的时候session表的next位置有上次使用的session_index，在删表时未清除。
    > 修改驱动代码，在建表时对session表写操作增加对next字段的写操作。
    > `ASIC_MEMORY_WRITE(((__u32 *)SESSION_ENTRY(ops, hsession_index)) + 7 , ((( __u32 *)half_session_entry) + 7 ) , 4 , ASIC_SESSION_API);`

- 在打pktgen的过程中，ftp传文件快转速率在(5-40)Mbps，的带宽跳变，最终结果文件可以正常传输。
