
//-----------------------------------------------------------------------------
//ITM_dlsms.inc, created 2016-05-16
// 
//-----------------------------------------------------------------------------


#IFDEF NOT __INC_ITM_UTIL__ THEN
#DEFINE __INC_ITM_UTIL__
INCLUDE rtcu.inc
INCLUDE math.inc
INCLUDE .\itm_config.inc   


VAR
   GWprm          : sockGetGWParm;
   clockdisplay : clockLinsecToTime; 
   timeFromGSM : BOOL;
END_VAR;

// Usefull functions
FUNCTION setINT;
VAR_INPUT
   adr : PTR;
   v   : INT;
END_VAR;
   memcpy(dst := adr + 1, src := ADDR(v)    , len := 1);
   memcpy(dst := adr    , src := ADDR(v) + 1, len := 1);
END_FUNCTION;
FUNCTION getINT;
VAR_INPUT
   dest : PTR;
   b0 : sint;
   b1 : sint;
END_VAR;
   memcpy(dst := dest , src := ADDR(b0) , len := 1);
   memcpy(dst := dest +1   , src := ADDR(b1), len := 1);
END_FUNCTION;
FUNCTION getFLOAT;
VAR_INPUT
   dest : PTR;
   b0 : sint;
   b1 : sint;
   b2 : sint;
   b3 : sint;
   debug : BOOL := False;
END_VAR;
   if debug then DebugMsg(message:="b0=" + sintToStr(v:=b0) + ", b1=" + sintToStr(v:=b1) + ", b2=" + sintToStr(v:=b2) + ", b3=" + sintToStr(v:=b3)); end_if;
   memcpy(dst := dest , src := ADDR(b0) , len := 1);
   memcpy(dst := dest +1   , src := ADDR(b1), len := 1);
   memcpy(dst := dest +2   , src := ADDR(b2), len := 1);
   memcpy(dst := dest +3   , src := ADDR(b3), len := 1);
END_FUNCTION;
FUNCTION getDINT;
VAR_INPUT
   dest : PTR;
   b0 : sint;
   b1 : sint;
   b2 : sint;
   b3 : sint;
   debug : BOOL := False;
END_VAR;
   if debug then DebugMsg(message:="b0=" + sintToStr(v:=b0) + ", b1=" + sintToStr(v:=b1) + ", b2=" + sintToStr(v:=b2) + ", b3=" + sintToStr(v:=b3)); end_if;
   memcpy(dst := dest , src := ADDR(b0) , len := 1);
   memcpy(dst := dest +1   , src := ADDR(b1), len := 1);
   memcpy(dst := dest +2   , src := ADDR(b2), len := 1);
   memcpy(dst := dest +3   , src := ADDR(b3), len := 1);
END_FUNCTION;
FUNCTION Int2Str : String;
VAR_INPUT
   val : MANDATORY int;
   len : int := 2;
END_VAR;

VAR
   tmpStr : STRING;
END_VAR;

   tmpStr := "00000000" + intToStr(v:=val);
   Int2Str := strRight(str:=tmpStr,length:=len);
END_FUNCTION; 
FUNCTION SInt2Str : String;
VAR_INPUT
   val : MANDATORY sint;
   len : int := 2;
END_VAR;

VAR
   tmpStr : STRING;
END_VAR;

   tmpStr := "00000000" + sintToStr(v:=val);
   SInt2Str := strRight(str:=tmpStr,length:=len);
END_FUNCTION; 
FUNCTION BoolToStr : STRING;
VAR_INPUT
   v  :  bool  := FALSE;
END_VAR;
   if v then 
      BoolToStr := "1";
   else
      BoolToStr := "0";
   end_IF;
END_FUNCTION;

FUNCTION setConfigGW : INT;
VAR_INPUT
   GWEnable   :  BOOL     := False;
   GWIP     :  STRING   := "";
   GWPort   :  DINT   :=0;
   GWKey    :  STRING   := "";
   CryptKey  : ARRAY[1..16] OF SINT;
   debug    :  BOOL     := False;
END_VAR;
VAR
   parmReset : BOOL     := False;

END_VAR;
   DebugMsg(message:=BoolToStr(v:=GWEnable)+";"+GWIP+";"+dintToStr(v:=GWPort)+";"+GWKey);
   
   IF GWprm.gwEnabled <> GWEnable THEN
      sockSetGWParm(gwEnabled := GWEnable, GWIP := GWIP, GWPort := GWPort, GWKey := GWKey, CryptKey := ADDR(CryptKey));
      parmReset := TRUE;
   END_IF;

   IF parmReset THEN
      // Reset device before the changes are used
      boardReset();
   END_IF;
   setConfigGW := int(GWEnable);
END_FUNCTION; 

FUNCTION getConfigGW : STRING;
VAR
   retVal : STRING     := "";
END_VAR;
   GWprm();
   retVal := intToStr(v:=int(GWprm.gwEnabled))+",";
   retVal := retVal + GWprm.GWIP + ",";
   retVal := retVal + dintToStr(v:=GWprm.GWPort) + ",";
   retVal := retVal + GWprm.GWKEY;
   getConfigGW := retVal;
END_FUNCTION; 

FUNCTION setAPN : INT;
VAR_INPUT
      apn : STRING;
END_VAR;
VAR
      retval :INT;
END_VAR;
   gprsClose ();
   retval := netSetMobileParam (APN := apn);
   gprsOpen();
   setAPN := retval;
END_FUNCTION;

FUNCTION StrToBool : BOOL;
var_INPUT
   str   : STRING :="";
END_VAR;
   if str="1" or str="True" or str="ON" or str="on" or str="On" OR str="TRUE" then
      StrToBool := True;
   else
      StrToBool := False;
   end_if;
END_FUNCTION;

function DigiToHex:STRING;
var_input
   v : int;
   debug : bool :=False;
end_var;
var
     
   tmpStr : STRING;
   
end_var;
   if v<10 then
      tmpStr := intToStr(v:=v);
   else
      case v of
         10: tmpStr := "A";
         11: tmpStr := "B";
         12: tmpStr := "C";
         13: tmpStr := "D";
         14: tmpStr := "E";
         15: tmpStr := "F";
      end_case;
   END_IF;
   
   DigiToHex := tmpStr;
END_FUNCTION; 

function SintToHex:STRING;
var_input
   v : sint;
   debug : bool :=False;
   
end_var;
var
   t : int := 0;  
   tmpint :int :=0;
   tmpStr : STRING;
   
end_var;
   t := int(v);
   if t<0 then
      t:= 128 + (t and 16#7F);
    END_IF;
      
   tmpint := t/16;
   tmpStr := DigiToHex(v:=tmpint);
   tmpint := t MOD 16;
   tmpStr := tmpStr + DigiToHex(v:=tmpint);
   SintToHex := tmpStr;
END_FUNCTION;
function HexToSInt:sint;
var_input
   str : STRING;
   debug : bool :=False;
end_var;
var
   tmpSint :sint :=0;
end_var;
   tmpSint := strToSint(str:=str);
   if str="a" or str="A" then tmpSint := 10;
   elsif str="b" or str="B" then tmpSint := 11;
   elsif str="c" or str="C" then tmpSint := 12;
   elsif str="d" or str="D" then tmpSint := 13;
   elsif str="e" or str="E" then tmpSint := 14;
   elsif str="f" or str="F" then tmpSint := 15;
   end_if;
   if debug then DebugFmt(message:="HexToSInt ("+str+")=\1",v1 := tmpSint);end_if;
   HexToSInt := tmpSint;
end_function;
FUNCTION linsecToDateStr:STRING;
VAR_INPUT
   linsec   : DINT;
END_VAR;
VAR
   tmpString   : STRING;
END_VAR;
   clockdisplay(Linsec:=linsec);
   tmpString := SInt2Str(val:=clockdisplay.Day) + "/" + SInt2Str(val:=clockdisplay.Month) + "/" + intToStr(v:=clockdisplay.Year);
   linsecToDateStr := tmpString;
END_FUNCTION;
function EraseArray:sint;
var_input
   dst : MANDATORY PTR;
   len : int := 0;
end_var;
var
   tmpSint : ARRAY[0..49] OF SINT ;
   i : int :=0;
end_var;
   for i:=0 to 49 do
         tmpSint[i] := 0;
   end_for;   
   memcpy(src:=ADDR(tmpSint),dst:=dst,len:=len);
   
end_function;

FUNCTION linsecToTimeStr:STRING;
VAR_INPUT
   linsec   : DINT;
END_VAR;
VAR
   tmpString   : STRING;

END_VAR;
   clockdisplay(Linsec:=linsec);
   tmpString := SInt2Str(val:=clockdisplay.Hour) + ":" + SInt2Str(val:=clockdisplay.Minute) + ":" + SInt2Str(val:=clockdisplay.Second);
   linsecToTimeStr := tmpString;
END_FUNCTION;

function updateTime
VAR_INPUT
   
   debug : BOOL := false;
END_VAR;
var
   time : dint;
end_var;
   time := gsmNetworkTime();
   IF time > 0 THEN
      time := time + int(cfg.timeZone) * 3600 ; 
      clockSet(linsec := time);
      if debug then DebugMsg(message:="Time updated to "+linsecToDateStr(linsec:=time) +" "+ linsecToTimeStr(linsec:=time) + " tz = " + sintToStr(v:=cfg.timeZone)); end_if;
   ELSE
      if debug then DebugMsg(message:="Time error from GSM "+dintToStr(v:=time)); end_if;
   END_IF;

end_function;
#END_IF;
