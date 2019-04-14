// Equipment outputs

#IFDEF NOT __INC_MQTT_CMD__ THEN
#DEFINE __INC_MQTT_CMD__

INCLUDE .\itm_mqtt.inc

FUNCTION checkMqttCmd;
#DEFINE CST_CMD_PORT_FOLDER "PORT"
#DEFINE CST_CMD_PORT0_FOLDER "PORT0"
#DEFINE CST_CMD_PORT1_FOLDER "PORT1"
#DEFINE CST_CMD_READ "READ"
#DEFINE CST_CMD_WRITE "WRITE"

#DEFINE CST_REG_PASSWORD         "PASSWORD"
#DEFINE CST_REG_READCONF         "READCONF"
#DEFINE CST_REG_READONECONF      "READONECONF"

#DEFINE CST_REG_READALL          "READALL"

#DEFINE CST_REG_REGISTER_STRING  "HRS"
#DEFINE CST_REG_REGISTER_INT     "HR"
#DEFINE CST_REG_REGISTER_INT32   "HRI"
#DEFINE CST_REG_REGISTER_INT64   "HRI_64"
#DEFINE CST_REG_REGISTER_FLOAT   "HRF"
#DEFINE CST_REG_REGISTER_RAW     "RAW"


#DEFINE CST_SEQUENCENUMBER_SEPARATOR ","
#DEFINE CST_TAGPATH_SEPARATOR "/"
#DEFINE CST_TAGCMD_SEPARATOR ":"
#DEFINE CST_CMDPARAMS_SEPARATOR "="
VAR_INPUT
   debug : BOOL := True;
END_VAR;
VAR
   
   send : bool;
   tagName : STRING;
   tagPath : STRING;
   tagValue : STRING;
   tagFolder : STRING;
   seqNumber : STRING;
   payload : STRING;
   cmd : STRING;
   params : STRING;
   reg : STRING;
   regValue : STRING;

   int32 : dint;
   tmpFloat : float;
   
   
END_VAR;
   send := False;
   
   // Extract all fields
   tagPath := lastCMD.name;
   tagFolder :=  strToken(str:=tagPath, delimiter:=CST_TAGPATH_SEPARATOR, index:=1);
   tagName := strToken(str:=tagPath, delimiter:= CST_TAGPATH_SEPARATOR, index:=2);
   tagValue := lastCMD.strval;
   seqNumber := strToken(str:=tagValue, delimiter:= CST_SEQUENCENUMBER_SEPARATOR, index:=1);
   payload := strToken(str:=tagValue, delimiter:= CST_SEQUENCENUMBER_SEPARATOR, index:=2);
   cmd := strToken(str:=payload, delimiter:= CST_TAGCMD_SEPARATOR, index:=1);
   params := strToken(str:=payload, delimiter:= CST_TAGCMD_SEPARATOR, index:=2);
   reg := strToken(str:=params, delimiter:= CST_CMDPARAMS_SEPARATOR, index:=1);
   regValue := strToken(str:=params, delimiter:= CST_CMDPARAMS_SEPARATOR, index:=2);

   IF debug=True THEN
      DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"tagPath : "+tagPath);
      DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"tagFolder : "+tagFolder);
      DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"tagName : "+tagName);
      DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"tagValue : "+tagValue);
      DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"cmd : "+cmd);
      DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"params : "+params);
      DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"reg : "+reg);
      DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"regValue : "+regValue);
   END_IF;
   
   
 
   // COM_PORT0/1 ? IF not return
   IF  tagFolder <> CST_CMD_PORT0_FOLDER AND
       tagFolder <> CST_CMD_PORT1_FOLDER THEN
      IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"non CMD PORT x : "+cmd);END_IF;
      RETURN;
   ELSE
      mqtt_ModBusCmd.port := strToSint(str:=strRight(str:=tagFolder ,length :=1));
      IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"CMD PORT "+ intToStr(v:=mqtt_ModBusCmd.port));END_IF;
   END_IF;
   
   // READ/WRITE COMMAND ? IF not return 
   IF tagName = CST_CMD_READ THEN 
      IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"READ CMD : "+params);END_IF;
      mqtt_ModBusCmd.func := CST_MODBUSCMD_READ_HR;
   ELSIF tagName =  CST_CMD_WRITE THEN 
      IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"WRITE CMD : "+params);END_IF;
      mqtt_ModBusCmd.func := CST_MODBUSCMD_WRITE_HR;
   ELSE
      IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"non READ/WRITE CMD : "+tagName);END_IF;
      RETURN;
   END_IF;
   mqtt_ModBusCmd.seqNumber := strToSint(str:=seqNumber);
   // Set Password : lenght 8 char, complete by space IF necessary
   IF cmd = CST_REG_PASSWORD THEN 
      IF mqtt_ModBusCmd.func = CST_MODBUSCMD_WRITE_HR THEN
         mqtt_ModBusCmd.type := 16#A;// #DEFINE CST_TYPE_STR          16#A 
         mqtt_ModBusCmd.reg := 16#0F00;
         params :=strLeft(str:= params+"        ", length:=8);
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"PASSWORD params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := strLen(str:=params)/2 ;
         strToMemory(dst:=ADDR(mqtt_ModBusCmd.values), str:=params, len:=strLen(str:=params));
         send := True;
         
      ELSE
         send := False;
      END_IF;
   END_IF;

   IF cmd = CST_REG_READCONF THEN 
      IF mqtt_ModBusCmd.func = CST_MODBUSCMD_WRITE_HR THEN
         send := False;
      ELSIF  mqtt_ModBusCmd.func = CST_MODBUSCMD_READ_HR THEN
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"READ CONF");END_IF; 
         mqtt_ModBusCmd.type := 16#10;// #DEFINE CST_TYPE_READ_CONF          16#10 
         send := True;
      END_IF;
   END_IF;
   IF cmd = CST_REG_READONECONF THEN 
      IF mqtt_ModBusCmd.func = CST_MODBUSCMD_WRITE_HR THEN
         send := False;
      ELSIF  mqtt_ModBusCmd.func = CST_MODBUSCMD_READ_HR THEN
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"READ ONE CONF " + reg);END_IF; 
         mqtt_ModBusCmd.type := 16#12;// #DEFINE CST_TYPE_READ_ONECONF          16#10
         mqtt_ModBusCmd.reg := strToInt(str:=reg);
         send := True;
      END_IF;
   END_IF;

   IF cmd = CST_REG_READALL THEN 
      IF mqtt_ModBusCmd.func = CST_MODBUSCMD_WRITE_HR THEN
         send := False;
      ELSIF  mqtt_ModBusCmd.func = CST_MODBUSCMD_READ_HR THEN
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"READ ALL");END_IF; 
         mqtt_ModBusCmd.type := 16#11;// #DEFINE CST_TYPE_READ_ALL         16#11
         send := True;
      END_IF;
   END_IF;

   
   // Set Register : RRRRR=vvvv.vv
   // RRRRR = register  (5 chars decimal)
   // vvvv.vv = value   (x chars by type)
   
   // Read Register : RRRRR=lll
   // RRRRR = register  (5 chars decimal)
   // lll = lenght   (x chars by type) 
   
//#DEFINE CST_TYPE_UINT8        16#0 

//#DEFINE CST_TYPE_DATABLOCK    16#F

   // lll = nb char  modbus length = nbChar /2
   IF cmd = CST_REG_REGISTER_STRING THEN 
      IF mqtt_ModBusCmd.func = CST_MODBUSCMD_WRITE_HR THEN
         mqtt_ModBusCmd.type := 16#A;// #DEFINE CST_TYPE_STR          16#A 
         mqtt_ModBusCmd.reg := strToInt(str:=reg);
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Reg " + reg +" params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := (strLen(str:=regValue)+1)/2 ;
         strToMemory(dst:=ADDR(mqtt_ModBusCmd.values), str:=params, len:=strLen(str:=params));
         send := True;
      ELSIF  mqtt_ModBusCmd.func = CST_MODBUSCMD_READ_HR THEN
         mqtt_ModBusCmd.type := 16#A;// #DEFINE CST_TYPE_STR          16#A 
         mqtt_ModBusCmd.reg := strToInt(str:=reg);
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Reg " + reg +" params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := strToInt(str:=regValue)+1/2 ;
         strToMemory(dst:=ADDR(mqtt_ModBusCmd.PublishName), str:=reg, len:=strLen(str:=reg)); 
         send := True;
      END_IF;
   END_IF;
   IF cmd = CST_REG_REGISTER_RAW THEN 
      IF mqtt_ModBusCmd.func = CST_MODBUSCMD_WRITE_HR THEN
         mqtt_ModBusCmd.type := 16#E;// #DEFINE CST_TYPE_RAW          16#E
         mqtt_ModBusCmd.reg := strToInt(str:=reg);
         mqtt_ModBusCmd.length := (strLen(str:=regValue)+1)/4 ;
         DebugMsg(message:=" Length = " + intToStr(v:=mqtt_ModBusCmd.length));
         
         MBCmdStrValueToMBValues(dst := ADDR(mqtt_ModBusCmd.values), HexStr:=regValue, len := strLen(str:=regValue));
         DebugMsg(message:=" v 0 = " + intToStr(v:=mqtt_ModBusCmd.values[0]));
         DebugMsg(message:=" v 1 = " + intToStr(v:=mqtt_ModBusCmd.values[1]));
        // strToMemory(dst:=ADDR(mqtt_ModBusCmd.values), str:=params, len:=strLen(str:=params));
         send := True;
      ELSIF  mqtt_ModBusCmd.func = CST_MODBUSCMD_READ_HR THEN
         mqtt_ModBusCmd.type := 16#E;// #DEFINE CST_TYPE_RAW          16#E
         mqtt_ModBusCmd.reg := strToInt(str:=reg);
         
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Reg " + reg +" params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := strToInt(str:=regValue) ;
         strToMemory(dst:=ADDR(mqtt_ModBusCmd.PublishName), str:=reg, len:=strLen(str:=reg)); 
         send := True;
      END_IF;
   END_IF;
   IF cmd = CST_REG_REGISTER_INT THEN 
      IF mqtt_ModBusCmd.func = CST_MODBUSCMD_WRITE_HR THEN
         mqtt_ModBusCmd.type := 16#1;//    #DEFINE CST_TYPE_UINT16       16#1 
         mqtt_ModBusCmd.reg := strToInt(str:=reg);
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Reg " + reg +" params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := 1 ;
         mqtt_ModBusCmd.values[0]:=strToInt(str:=regValue);
         send := True;
      ELSIF  mqtt_ModBusCmd.func = CST_MODBUSCMD_READ_HR THEN
         mqtt_ModBusCmd.type := 16#1;//    #DEFINE CST_TYPE_UINT16       16#1 
         mqtt_ModBusCmd.reg := strToInt(str:=reg);
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Reg " + reg +" params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := 1 ;
         send := True;
      END_IF;
   END_IF;
   
   IF cmd = CST_REG_REGISTER_INT32 THEN 
      IF mqtt_ModBusCmd.func  = CST_MODBUSCMD_WRITE_HR THEN
         mqtt_ModBusCmd.type := 16#2;//    #DEFINE CST_TYPE_UINT32       16#2 
         mqtt_ModBusCmd.reg  := strToInt(str:=reg);
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Reg " + reg +" params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := 2 ;
         int32 := strToDint(str:=regValue);
         memcpy(dst:=ADDR(mqtt_ModBusCmd.values), src:=addr(int32), len:=sizeof(int32));
         send := True;
      ELSIF  mqtt_ModBusCmd.func = CST_MODBUSCMD_READ_HR THEN
         mqtt_ModBusCmd.type := 16#2;//    #DEFINE CST_TYPE_UINT32       16#2 
         mqtt_ModBusCmd.reg  := strToInt(str:=reg);
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Reg " + reg +" params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := 2 ;
         send := True;
      END_IF;
   END_IF;
   
   IF cmd = CST_REG_REGISTER_FLOAT THEN 
      IF mqtt_ModBusCmd.func  = CST_MODBUSCMD_WRITE_HR THEN
         mqtt_ModBusCmd.type := 16#3;//    #CST_TYPE_REAL32       16#3 
         mqtt_ModBusCmd.reg  := strToInt(str:=reg);
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Reg " + reg +" params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := 2 ;
         tmpFloat := strToFloat(str:=regValue);
         memcpy(dst:=ADDR(mqtt_ModBusCmd.values), src:=addr(tmpFloat), len:=sizeof(tmpFloat));
         send := True;
      ELSIF  mqtt_ModBusCmd.func = CST_MODBUSCMD_READ_HR THEN
         send := False;
      END_IF;
   END_IF;
   
   /* TO COMPLETE
   IF cmd = CST_REG_REGISTER_DOUBLE THEN 
      IF mqtt_ModBusCmd.func  = CST_MODBUSCMD_WRITE_HR THEN
         mqtt_ModBusCmd.type := 16#4//    ##DEFINE CST_TYPE_INTREAL       16#4 
         mqtt_ModBusCmd.reg  := reg;
         IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Reg " + intToStr(v:=reg) +" params = ["+params+"]");END_IF; 
         mqtt_ModBusCmd.length := 4 ;
         tmpFloat := strToFloat(str:=params);
         strToMemory(dst:=ADDR(mqtt_ModBusCmd.values), str:=addr(tmpFloat), len:=sizeof(tmpFloat));
         send := True;
      ELSIF  mqtt_ModBusCmd.func = CST_MODBUSCMD_READ_HR THEN
         send := False;
      END_IF;
   END_IF;
   */
   
   IF send=True THEN
      IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"SEND TO CHANNEL ");END_IF;
      // Write data to channel
      chWrite_retVal := chWrite(ch:=mqtt_channel,msg:=addr(mqtt_ModBusCmd),len:=sizeof(mqtt_ModBusCmd));
   ELSE
      IF debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"NO SENDING, tagValue = "+tagValue);END_IF;   
   END_IF;
   
END_FUNCTION;

#END_IF