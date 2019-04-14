//-----------------------------------------------------------------------------
//ITM_sms.inc, created 2018-05-31
// 
//-----------------------------------------------------------------------------


#IFDEF NOT __INC_ITM_SMS__ THEN
#DEFINE __INC_ITM_SMS__
INCLUDE rtcu.inc
INCLUDE math.inc
INCLUDE .\itm_config.inc   
INCLUDE .\itm_util.inc

#DEFINE CST_DELIMITER_SERIAL ";"
#DEFINE CST_DELIMITER_PARAM   ","

// SMS FORMAT IS : SERIALNUMBEROFEDGEWARE;COMMAND
//send SMS to sim number :
//1234567890;RESET

#DEFINE CST_MORE_SPACE "                                                                    "

#DEFINE CST_SMS_PARAMETER_FTP                 "setFTP"
#DEFINE CST_SMS_PARAMETER_FTPURL             "setFtpUrl"
#DEFINE CST_SMS_PARAMETER_FTPUSER            "setFtpUser"
#DEFINE CST_SMS_PARAMETER_FTPPASS            "setFtpPass"
#DEFINE CST_SMS_PARAMETER_ALERTPHONENUMBER   "setAlertPhoneNumber"
#DEFINE CST_SMS_PARAMETER_ALERTENABLE        "setAlertEnable"
#DEFINE CST_SMS_PARAMETER_APN                "setApn"
#DEFINE CST_SMS_PARAMETER_GW                 "setGw"
#DEFINE CST_SMS_PARAMETER_EVENTLEVEL         "setRv"
#DEFINE CST_SMS_PARAMETER_TIMEZONE         "setTz"
#DEFINE CST_SMS_PARAMETER_SETTIME         "setTime" 
//SetTime=YYYYMMDDHHMM

#DEFINE CST_SMS_COMMAND_STATUS         "getStatus"
//#DEFINE CST_SMS_COMMAND_GPRS           "gprs"
#DEFINE CST_SMS_COMMAND_ERROR          "getError"
#DEFINE CST_SMS_COMMAND_RESET          "reset"
#DEFINE CST_SMS_COMMAND_GSMSIGNAL      "getGsmSignal"
#DEFINE CST_SMS_COMMAND_GPRSSTATUS     "getGprsStatus"
#DEFINE CST_SMS_COMMAND_GWSTATUS       "getGwStatus"
#DEFINE CST_SMS_COMMAND_GWSETUP        "getGwSetup"
#DEFINE CST_SMS_COMMAND_FTPSTATUS      "getFtpStatus"
#DEFINE CST_SMS_COMMAND_FTPSETUP       "getFtpSetup"
#DEFINE CST_SMS_COMMAND_FTPTEST        "ftpTest"
#DEFINE CST_SMS_COMMAND_FTPSENT        "ftpSent"
#DEFINE CST_SMS_COMMAND_ALERTSTATUS    "getAlertStatus"
#DEFINE CST_SMS_COMMAND_SDSTATUS       "getSdStatus"
#DEFINE CST_SMS_COMMAND_GETNEWCONFIG   "getNewConfig"
#DEFINE CST_SMS_COMMAND_ACTUALCONFIG   "ActualConfig"
#DEFINE CST_SMS_COMMAND_TIMEFROMGSM    "getTimeFromGSM"
#DEFINE CST_SMS_COMMAND_PUSHFILES      "pushFiles"




THREAD_BLOCK thEventSMS;

VAR
   strSerial : String :="";
 
   
   
   incoming    : gsmIncomingSMS;
   tmpString   : STRING    := "";
   tmpParam    : STRING    := "";
   tmpParam1   : STRING    := "";
   tmpParam2   : STRING    := "";
   tmpParam3   : STRING    := "";
   tmpBool     : BOOL     := False;
   answer      : STRING    := "";
   debug       : BOOL      := True;
   savePrm     : BOOL      := False;
   tmpInt      : INT       := 0;
   y : INT;
   m : SINT;
   d :SINT;
   h :SINT;
   mn : SINT;
END_VAR;
   
   strSerial := dintToStr(v:=boardSerialNumber());
   
   WHILE TRUE DO
      incoming();
      IF incoming.status = 1 THEN
         answer := "";
         savePrm := False;
         displayXY(x:=1, y:=1);
         displayString(message:=incoming.phonenumber+"                         ");
         displayXY(x:=1, y:=2);
         displayString(message:=incoming.message+"                              ");
         tmpString := strToken(str:=incoming.message, delimiter :=CST_DELIMITER_SERIAL, index :=1);
         if tmpString = strSerial THEN
            if debug then DebugMsg(message :="SMS Received new message with good serial id"); end_if;
            if debug then DebugMsg(message :="FROM " + incoming.phonenumber); end_if;
            tmpString := strToken(str:=incoming.message, delimiter :=CST_DELIMITER_SERIAL, index :=2);
            if debug then DebugMsg(message :="  Message is : " + tmpString); end_if;
            

            IF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_PARAMETER_APN)) ,str2:= CST_SMS_PARAMETER_APN)=0 THEN
               if debug THEN DebugMsg(message:="PARAMETER APN"); END_IF;
               tmpParam :=  strToken(str:=tmpString, delimiter :="=", index :=2);
               
               if setAPN(apn:=tmpParam)=0 then
                  answer := CST_SMS_PARAMETER_APN + " Done";
               else
                  answer := CST_SMS_PARAMETER_APN + " Error";
               end_if;
        
            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_PARAMETER_GW)) ,str2:= CST_SMS_PARAMETER_GW)=0 THEN
               if debug THEN DebugMsg(message:="PARAMETER GW "); END_IF;
               tmpParam := strToken(str:=tmpString, delimiter :="=", index :=2);
               tmpBool := StrToBool(str:=strToken(str:=tmpParam, delimiter :=CST_DELIMITER_PARAM, index :=1));
               tmpParam1 := strToken(str:=tmpParam, delimiter :=CST_DELIMITER_PARAM, index :=2);
               tmpParam2 := strToken(str:=tmpParam, delimiter :=CST_DELIMITER_PARAM, index :=3);
               tmpParam3 := strToken(str:=tmpParam, delimiter :=CST_DELIMITER_PARAM, index :=4);
               DebugMsg(message:=tmpParam+" -> " + BoolToStr(v:=tmpBool)+CST_DELIMITER_PARAM+tmpParam1+CST_DELIMITER_PARAM+dintToStr(v:=strToDint(str:=tmpParam2))+CST_DELIMITER_PARAM+tmpParam3);
               answer := CST_SMS_PARAMETER_GW + " Done : " +intToStr(v:=setConfigGW(GWEnable:=tmpBool, GWIP:=tmpParam1, GWPort :=strToDint(str:=tmpParam2), GWKey := tmpParam3, Debug:=TRUE));
               
            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_PARAMETER_SETTIME)) ,str2:= CST_SMS_PARAMETER_SETTIME)=0 THEN
               // SetTime=YYYYMMDDHHMM
               if debug THEN DebugMsg(message:="SET TIME "); END_IF;
               tmpParam := strToken(str:=tmpString, delimiter :="=", index :=2);
               DebugMsg(message:=tmpParam + " -> " + strLeft(str:=tmpParam, length :=4));
               y := strToInt(str:=strLeft(str:=tmpParam, length :=4));
               m := strToSint(str:=strMid(str:=tmpParam, start := 5, length :=2));
               d := strToSint(str:=strMid(str:=tmpParam, start := 7, length :=2));
               h := strToSint(str:=strMid(str:=tmpParam, start := 9, length :=2));
               mn := strToSint(str:=strMid(str:=tmpParam, start := 11, length :=2));
               clockSet(Year := y, Month := m, Day := d, Hour:=h, Minute:=mn);

               answer := CST_SMS_PARAMETER_SETTIME + " Done " + intToStr(v:=y) + "-" + sintToStr(v:=m) + "-" + sintToStr(v:=d) + " " + sintToStr(v:=h) + ":" + sintToStr(v:=mn);
            

            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_COMMAND_STATUS)) ,str2:= CST_SMS_COMMAND_STATUS)=0 THEN
               if debug THEN DebugMsg(message:="COMMAND STATUS"); END_IF;
               answer := "Temp = " + floatToStr(v:= float(boardTemperature()) / 100.0) + "°C;";
               answer := answer + " Free Memory = " + dintToStr(v:= fsMediaFree(media := 0)) + "KB;";
               answer := answer + " Bat = " + sintToStr(v:= batPowerLevel()) + " Level;";
            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_COMMAND_GWSTATUS)) ,str2:= CST_SMS_COMMAND_GWSTATUS)=0 THEN
               if debug THEN DebugMsg(message:="COMMAND GW STATUS"); END_IF;
               answer := "GATEWAY : Enabled=" + BoolToStr(v:=gwEnabled()) + " Connected=" + BoolToStr(v:=gwConnected());
            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_COMMAND_GWSETUP)) ,str2:= CST_SMS_COMMAND_GWSETUP)=0 THEN
               if debug THEN DebugMsg(message:="COMMAND GW SETUP"); END_IF;
               answer := "GATEWAY : Setup = " + getConfigGW();
            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_COMMAND_GPRSSTATUS)) ,str2:= CST_SMS_COMMAND_GPRSSTATUS)=0 THEN
               if debug THEN DebugMsg(message:="COMMAND GPRS STATUS"); END_IF;
               answer := "GPRS : Connected=" + BoolToStr(v:=gprsConnected());
            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_COMMAND_RESET)) ,str2:= CST_SMS_COMMAND_RESET)=0 THEN
               if debug THEN DebugMsg(message:="COMMAND RESET"); END_IF;
               boardReset();
            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_COMMAND_GSMSIGNAL)) ,str2:= CST_SMS_COMMAND_GSMSIGNAL)=0 THEN
               if debug THEN DebugMsg(message:="COMMAND GSM SIGNAL"); END_IF;
               answer := "GSM Signal =  " + intToStr(v:=gsmSignalLevel()) + "%";
         
            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_COMMAND_TIMEFROMGSM)) ,str2:= CST_SMS_COMMAND_TIMEFROMGSM)=0 THEN
               if debug THEN DebugMsg(message:="COMMAND TIME FROM GSM"); END_IF;
               timeFromGSM := True;
               answer := "TIME UPDATED";

    
            ELSIF strCompare(str1:=strLeft(str:=tmpString,length := strLen(str:=CST_SMS_COMMAND_SDSTATUS)) ,str2:= CST_SMS_COMMAND_SDSTATUS)=0 THEN
               if debug THEN DebugMsg(message:="COMMAND SD STATUS"); END_IF;
               answer := "Size = " + dintToStr(v:= fsMediaSize (media:=0)) + "KB;";
               answer := answer + " Free Size = " + dintToStr(v:= fsMediaFree (media:=0)) + "KB";
            END_IF;
            if strLen(str := answer) >0 then
               gsmSendSMS(phonenumber := incoming.phonenumber, message := strSerial + CST_DELIMITER_SERIAL + answer);
            end_if;
         END_IF;
         
      END_IF;
         

   END_WHILE;
END_THREAD_BLOCK;


#END_IF;