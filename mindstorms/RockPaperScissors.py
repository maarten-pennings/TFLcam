import hub
from sys import exit
from utime import sleep_ms, ticks_ms, ticks_diff

class TFLcam :
    # Configures `port` for the TFLcam sensor
    def __init__(self,port) :
        # Claim port
        if not port in "ABCDEF" :
            raise Exception("port must be A,B,C,D,E, or F")
        self.port = eval("hub.port."+port)
        # Configure port as 115200 full duplex serial
        self.port.mode( hub.port.MODE_FULL_DUPLEX )
        sleep_ms(100)
        self.port.baud(115200)
        sleep_ms(100)
        # Initialize receive buffer
        self.rxbuf = b""
        # Sync
        self.cmd("") # clear pending commands
        self.cmd("echo enabled") # clear disabled
        self.cmd("echo [sync]","[sync]\r\n>> ")
        self.cmd("mode idle") # reset opmode
    # Sends command ucmd to TFLcam, waits for an answer (containg usync) and returns the answer
    def cmd(self,ucmd,usync=">> ") :
        # Convert strings to bytes
        bcmd= ucmd.encode()
        bsync= usync.encode()
        self.port.write(bcmd+b"\n") # Send bytes with <CR> to command interpreter
        pos,tries = -1,0
        while (pos<0) and (tries<100):
            self.rxbuf += self.port.read(256)
            pos = self.rxbuf.find(bsync)
            sleep_ms(50)
            tries= tries+1
        if pos<0 :
            raise Exception("cmd(): sync not received ["+self.rxbuf.decode()+"]")
        result=self.rxbuf[:pos]
        self.rxbuf=self.rxbuf[pos+len(bsync):]
        try :
            result = result.decode() # Convert bytes to strings
        except :
            result = ""
        return 
    # Returns one line send by the TFLcam (or None)
    def readln(self) :
        self.rxbuf += self.port.read(256)
        pos = self.rxbuf.find(b"\n")
        if pos>=0 :
            result = self.rxbuf[:pos]
            self.rxbuf = self.rxbuf[pos+1:]
            return result.decode()
        return None
    # Returns class label send by TFLcam (or None)
    def readlbl(self) :
        line = self.readln()
        # Parse eg "predict: 1/paper"
        if line==None : return None
        words = line.split(" ")
        if len(words)!=2 : return None
        if words[0]!="predict:" : return None
        return words[1]

class Hand :
    # Configures `portlo` and `porthi` for the two motors of the hand
    def __init__(self,portlo,porthi) :
        # Claim portlo
        if not portlo in "ABCDEF" :
            raise Exception("portlo must be A,B,C,D,E, or F")
        self.portlo = eval("hub.port."+portlo+".motor")
        if self.portlo.__class__.__name__!="Motor":
            raise Exception(porthi+" has no motor")
        # Claim porthi
        if not porthi in "ABCDEF" :
            raise Exception("porthi must be A,B,C,D,E, or F")
        self.porthi = eval("hub.port."+porthi+".motor")
        if self.porthi.__class__.__name__!="Motor":
            raise Exception(porthi+" has no motor")
        # Startup animation
        hub.display.align(hub.RIGHT)
        self.rock()
        self.paper()
        self.scissors()
        self.none()
    def none(self):
        hub.display.show(hub.Image("00000:00000:09990:00000:00000")) # "-"
        # Leave motors where they are
    def rock(self):
        self.porthi.run_to_position(145)
        self.portlo.run_to_position(145)
        hub.display.show(hub.Image("09990:09009:09990:09090:09009")) # "R"
        sleep_ms(500) # motors run in background
    def paper(self):
        self.porthi.run_to_position(0)
        self.portlo.run_to_position(0)
        hub.display.show(hub.Image("09990:09009:09990:09000:09000")) # "P"
        sleep_ms(500) # motors run in background
    def scissors(self):
        self.porthi.run_to_position(145)
        self.portlo.run_to_position(0)
        hub.display.show(hub.Image("09990:90000:09990:00009:09990")) # "S"
        sleep_ms(500) # motors run in background
    def pose(self,lbl) :
        if lbl=="0/none" : self.none()
        if lbl=="1/paper" : self.paper()
        if lbl=="2/rock" : self.rock()
        if lbl=="3/scissors" : self.scissors()

print("START")
tflcam = TFLcam('A')
hand = Hand("B","D")

tflcam.cmd( "mode cont 2" )

last = ticks_ms()
while ticks_diff(ticks_ms(),last) < 30000 :
    lbl = tflcam.readlbl() 
    if lbl!=None : 
        print("  "+lbl)
        hand.pose(lbl)
        last = ticks_ms()

tflcam.port.write("mode idle\n")
print("STOP")
exit(0)
