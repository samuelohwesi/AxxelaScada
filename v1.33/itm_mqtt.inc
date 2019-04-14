//-----------------------------------------------------------------------------
//ITM_mqtt.inc, created 2018-05-31
// 
//-----------------------------------------------------------------------------


#IFDEF NOT __INC_ITM_MQTT__ THEN
#DEFINE __INC_ITM_MQTT__
INCLUDE rtcu.inc
INCLUDE math.inc
INCLUDE .\itm_util.inc   
INCLUDE .\itm_network.inc

INCLUDE .\outputs.inc


// *********************************
// MQTT
// *********************************

// DEBUG

#DEFINE CST_DEBUG_MQTT           1
#DEFINE CST_DEBUG_MQTT_PUBLISH   0
#DEFINE CST_DEBUG_MQTT_RECEPT    1
#DEFINE CST_DEBUG_MQTT_CONFIG    1
#DEFINE CST_DEBUG_MQTT_FORWARD   0

#DEFINE CST_PREFIX_DEBUG_MQTT "<* MQTT *> "


// CACHE
#DEFINE CST_MAX_STORE_AND_FORWARD 98

// DATA TYPE
#DEFINE CST_TYPE_SINT 0
#DEFINE CST_TYPE_INT 1
#DEFINE CST_TYPE_DINT 2
#DEFINE CST_TYPE_BOOL 3
#DEFINE CST_TYPE_FLOAT 4
#DEFINE CST_TYPE_STRING 5

STRUCT_BLOCK typeCMD
   name : STRING;
   type : SINT;
   fval : FLOAT := 0.0;
   dval : DINT := 0;
   ival : INT := 0;
   sval : SINT := 0;
   strval : STRING;
   bval : BOOL := FALSE;
   
END_STRUCT_BLOCK;

#DEFINE CST_MODBUSCMD_READ_HR 16#3
#DEFINE CST_MODBUSCMD_WRITE_HR 16#10

STRUCT_BLOCK ALIGN typeModBusCmd
   port : SINT;
   func : SINT;
   type : SINT;
   reg : INT;
   length : INT;
   values : ARRAY[0..50] OF INT;
   PublishName : ARRAY[0..30] OF SINT;
   seqNumber : SINT;
END_STRUCT_BLOCK;


VAR
   // Config


   brokerMQTT  : STRING :=  "m23.cloudmqtt.com";
   portMQTT    : int := 14920;
   userMQTT    : STRING := "mac";
   passMQTT  : STRING := "mac";
   clientidMQTT  : STRING := "edgeware";
   lwt_message : STRING;
   lwt_retained : Bool;

   
   // realtimedata
   handleMQTT        : INT := -1;
   payloadMQTT       : STRING;
   topicMQTT         : STRING;
   subTopicMQTT : STRING ;
   bufMQTT           : ARRAY [1..256] OF SINT;
   qosMQTT : SINT := 1;

   payloadReqMQTT : ARRAY [0..500] OF SINT;
   rxd : mqttReceive; 
   rc   : INT;
   
     // Store ans Forward
   MxForwardIndex : MUTEX;
   storeIndex : INT := 0;
   forwardIndex : INT := 0;
   
   lastCMD : typeCMD;

   mqtt_channel : CHANNEL;
   mqtt_ModBusCmd : typeModBusCmd;
   chRead_retVal : int;
   chWrite_retVal : int;
   
END_VAR;


INCLUDE .\mqtt_cmd.inc

FUNCTION configMQTT;

    // MQTT
  if CST_DEBUG_MQTT_CONFIG>0 THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Initialize MQTT..."); END_IF;
  
  // Model sparkplug : spBv1.0/France/DDATA/F85/
  clientidMQTT := strFormat(format := "EW-\4/", v4 := boardSerialNumber());
  topicMQTT := strFormat(format := "EW/\4/", v4 := boardSerialNumber());
  payloadMQTT := strFormat(format := "\4", v4 := boardSerialNumber());
  subTopicMQTT := strFormat(format := "spBv1.0/EW/DDATA/CMD/\4/#", v4 := boardSerialNumber());
  
  //topicMQTT := strFormat(format := "spBv1.0/France/DDATA/F85/EW/X400_\4/", v4 := boardSerialNumber());
  if CST_DEBUG_MQTT_CONFIG>0 THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"    --> clientidMQTT : " + clientidMQTT); END_IF;
  if CST_DEBUG_MQTT_CONFIG>0 THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"    --> Topic : " + topicMQTT); END_IF;
  if CST_DEBUG_MQTT_CONFIG>0 THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"    --> payloadMQTT : " + payloadMQTT); END_IF;
  if CST_DEBUG_MQTT_CONFIG>0 THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"    --> subTopicMQTT : " + subTopicMQTT); END_IF;
  
END_FUNCTION; 

FUNCTION connectMQTT;
VAR_INPUT
   iface : SINT;
END_VAR;

   if boardSerialNumber() = 31174303 THEN
      brokerMQTT     :=  "demo.it-mation.com";
      //brokerMQTT     :=  "192.168.0.1";
      portMQTT       := 1883;
      userMQTT       := "EW";
      passMQTT       := "password";
      clientidMQTT   := payloadMQTT;
      //handleMQTT := mqttOpen(ip := "server1.austral-concept.fr",  iface:= iface, port := 1883, username :="EW", password :="password", clientid := payloadMQTT);
      
      //DebugFmt(message := "MQTT server1.austral-concept.fr handle= \1", v1 := handleMQTT);
   ELSE
      brokerMQTT     := "userveprodsrv.westeurope.cloudapp.azure.com";
      portMQTT       := 1883;
      userMQTT       := "EW";
      passMQTT       := "oo3DdW2n";
      clientidMQTT := strFormat(format := "EW-\4/", v4 := boardSerialNumber());
      lwt_message  := " { $"Will$" : $"Will Message$", $"PollTime$" :"+dintToStr(v:=clockNow()+CST_DELTA_X32_UNIX)+"}";
      lwt_retained  := True;

      
      //handleMQTT := mqttOpen(ip := "userveprodsrv.westeurope.cloudapp.azure.com", iface:= iface, port := 1883, username :="EW", password :="oo3DdW2n", clientid := payloadMQTT);
      //DebugFmt(message := "MQTT userveprodsrv.westeurope.cloudapp.azure.com handle= \1", v1 := handleMQTT);
   END_IF; 

  handleMQTT := mqttOpen(  ip :=brokerMQTT, 
                           iface:= iface, 
                           port := portMQTT,
                           username :=userMQTT, 
                           password :=passMQTT, 
                           clientid := clientidMQTT,
                           lwt_message := lwt_message,
                           lwt_topic := topicMQTT,
                           lwt_retained := lwt_retained
                           );
                           
  if CST_DEBUG_MQTT_CONFIG>0 THEN DebugFmt(message := CST_PREFIX_DEBUG_MQTT+brokerMQTT + ":\1 (" + userMQTT +")    --> Handle= \2", v1:=portMQTT, v2 := handleMQTT);END_IF;
  if handleMQTT>-1 THEN
     rc := mqttSubscribe(handle := handleMQTT, qos := 2, topic := subTopicMQTT);
     if CST_DEBUG_MQTT_CONFIG>0 THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"      --> Subscribe : " + subTopicMQTT); END_IF;
  END_IF;

 
END_FUNCTION;

FUNCTION disconnectMQTT;

   if mqttConnected(handle:=handleMQTT) THEN 
      mqttClose(handle:=handleMQTT);
      handleMQTT := -1;
   END_IF;
   
END_FUNCTION;

FUNCTION decodeInt64:dint;
VAR_INPUT
      src: PTR;
END_VAR;
VAR
   i: sint;
   val : dint := 0;
   val1 : sint := 0;
END_VAR;
   for i := 1 TO 5 do
      memcpy(dst := ADDR(val1) , src := src+i , len := 1);
      if i=1 THEN
         val := val1 AND 16#7F;
      ELSE
         val := val OR shl32(in:=(val1 AND 16#7F),n:=7*(i-1));
      END_IF;
      if (val1 AND 16#80)=0 THEN EXIT;END_IF;
   END_FOR;
   decodeInt64 := val;
END_FUNCTION;

FUNCTION parseSpBv1CMD : BOOL;
VAR_INPUT
   debug : bool := true;
END_VAR;
VAR
   nbCar : sint;
  t : sint;
  type : sint;
  codage : sint;

END_VAR;
   nbCar := sint(payloadReqMQTT[10]);
   codage := payloadReqMQTT[nbCar+20+4];
   if debug THEN DebugMsg(message:=CST_PREFIX_DEBUG_MQTT+"nbCar = "+intToStr(v:=nbCar)); End_IF;
   if debug THEN  DebugMsg(message:=CST_PREFIX_DEBUG_MQTT+"codage = "+intToStr(v:=codage)); End_IF;
   
   lastCMD.name := strFromMemory(src := ADDR(payloadReqMQTT[11]), len := nbCar);//strMid(str:=payload,start:=12,length:=i);
   if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"name = "+lastCMD.name);END_IF;

   case codage OF
      16#50:
         DebugMsg(message:=CST_PREFIX_DEBUG_MQTT+"Type int64");
         lastCMD.type := CST_TYPE_DINT;
         lastCMD.dval := decodeInt64(src:=addr(payloadReqMQTT[nbCar+20+4]));
         lastCMD.ival := int(lastCMD.dval);
         if lastCMD.dval<>0 THEN lastCMD.bval := True; ELSE lastCMD.bval := False; END_IF;
         lastCMD.fval := float(lastCMD.dval);
         lastCMD.strval := dintToStr(v:=lastCMD.dval);
         
      16#70:
         DebugMsg(message:=CST_PREFIX_DEBUG_MQTT+"Type boolean");
         lastCMD.type := CST_TYPE_BOOL;
         if sint(payloadReqMQTT[nbCar+20+5])=1 THEN
            lastCMD.bval := True;
            lastCMD.ival := 1;
            lastCMD.dval := 1;
            lastCMD.fval := 1.0;
            lastCMD.strval := "True";
         else
            lastCMD.bval := False;
            lastCMD.ival := 0;
            lastCMD.dval := 0;
            lastCMD.fval := 0.0;
            lastCMD.strval := "False";
         end_if;
      16#65:
         DebugMsg(message:=CST_PREFIX_DEBUG_MQTT+"Type float");
         lastCMD.type := CST_TYPE_FLOAT;
         // Copy contents of one memory area to another area
         memcpy(dst:=ADDR(lastCMD.fval), src:=ADDR(payloadReqMQTT[nbCar+20+5]), len:=4);
         lastCMD.dval := dint(lastCMD.fval);
         lastCMD.ival := int(lastCMD.dval);
         if lastCMD.dval<>0 THEN lastCMD.bval := True; ELSE lastCMD.bval := False; END_IF;
         lastCMD.strval := floatToStr(v:=lastCMD.fval);
      16#69:
         DebugMsg(message:=CST_PREFIX_DEBUG_MQTT+"Type double");
      16#7A:
         DebugMsg(message:=CST_PREFIX_DEBUG_MQTT+"Type String");
         lastCMD.type := CST_TYPE_STRING;
         lastCMD.ival := int(payloadReqMQTT[nbCar+20+5]);
         if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"nbChar = "+intToStr(v:=lastCMD.ival));END_IF;
         lastCMD.dval := dint(lastCMD.ival);
         lastCMD.bval := False; 
         lastCMD.fval := float(lastCMD.ival);
         lastCMD.strval := strFromMemory(src:=ADDR(payloadReqMQTT[nbCar+20+6]), len:=lastCMD.ival);
      ELSE
         DebugMsg(message:=CST_PREFIX_DEBUG_MQTT+"Type unknown");
   END_CASE;
   if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"valeur = "+lastCMD.strval);END_IF;

   
//   deb := nbCar+26;
   
   
      
   
   parseSpBv1CMD := True;
END_FUNCTION;

FUNCTION buildPayLoadMQTT : STRING;
VAR_INPUT     
      tagName : STRING:="";
      tagValue : FLOAT := 0.0;
      tagUnit : STRING:= "";
      tagQuality : STRING := "GOOD";
END_VAR;

   if tagName ="" THEN
      buildPayLoadMQTT := "";
      RETURN;
   END_IF;
   buildPayLoadMQTT := " { $"Name$" :$""+tagName+"$", $"Unit$" :$""+tagUnit+"$", $"Value$" :"+floatToStr(v:=tagValue)+",$"Quality$":$""+tagQuality+"$"}";
   
END_FUNCTION;

FUNCTION publishMQTT;
VAR_INPUT     
      payload : String;
      topic : String;
END_VAR;
VAR
   
   retMQTT : INT;
END_VAR;
 
   strToMemory(dst := ADDR(bufMQTT), str := payload, len := strLen(str := payload));
   if CST_DEBUG_MQTT_PUBLISH>0 then DebugFmt(message := "Published = " +  topic + " - " + payload); END_IF;
   retMQTT := mqttPublish(
              handle   := handleMQTT,
              topic   := topic ,
              qos     := qosMQTT,
              retained := FALSE,
              data     := ADDR(bufMQTT),
              size     := strLen(str := payload)
             );
            IF CST_DEBUG_MQTT_PUBLISH>0 THEN
               IF retMQTT<0 THEN
                  DebugFmt(message := "retMQTT = \1", v1:=retMQTT);
               ELSE
                  DebugFmt(message := "Publication ok");
               END_IF;
            END_IF;
END_FUNCTION;

FUNCTION publishTagMQTT;
VAR_INPUT     
      tagName : STRING:="";
      tagValue : FLOAT := 0.0;
      tagUnit : STRING:= "";
      tagQuality : STRING := "GOOD";
END_VAR;
   // Read the name of a tag
   publishMQTT(topic:=topicMQTT,payload:=buildPayLoadMQTT(tagName:=tagName,tagValue:=tagValue,tagUnit:=tagUnit,tagQuality:=tagQuality));

END_FUNCTION;


FUNCTION storeMQTT;
VAR_INPUT
   payload : String;
   topic : String;
   
END_VAR;
   
   mxLock(mx:=MxForwardIndex);

   IF storeIndex=forwardIndex and mqttConnected(handle := handleMQTT) and OFF THEN
      publishMQTT(topic := topic, payload := payload);
   ELSE
      if CST_DEBUG_MQTT_FORWARD>0 then DebugFmt(message := "Store \1= " +  topic + " - " + payload, v1 := storeIndex); end_if;
      SaveStringF(index:=storeIndex+1, str := topic);
      SaveStringF(index:=storeIndex+2, str := payload);
      storeIndex := (storeIndex + 2) mod CST_MAX_STORE_AND_FORWARD;
      IF storeIndex=forwardIndex THEN
            forwardIndex := (forwardIndex + 2) MOD CST_MAX_STORE_AND_FORWARD;
      END_IF;
   END_IF;
   
   mxUnlock(mx:=MxForwardIndex);
   
END_FUNCTION;

THREAD_BLOCK thForwardMQTT;
VAR
      topic : STRING;
      payload  :  STRING;
END_VAR;
      
   

   WHILE On DO
      IF forwardIndex<>storeIndex THEN
         mxLock(mx:=MxForwardIndex);
            topic := LoadStringF(index:=forwardIndex+1);
            payload := LoadStringF(index:=forwardIndex+2);
            if CST_DEBUG_MQTT_FORWARD>0 then DebugFmt(message := "Forward \1= " +  topic + " - " + payload, v1 := forwardIndex); END_IF;

            publishMQTT( topic := topic, payload := payload);
            forwardIndex := (forwardIndex + 2) MOD CST_MAX_STORE_AND_FORWARD;
         mxUnlock(mx:=MxForwardIndex);
      END_IF;
   END_WHILE;
   
END_THREAD_BLOCK;

FUNCTION checkBuiltIn;
#DEFINE CST_BUILTIN_FOLDER "BUILT IN"
#DEFINE CST_ANALOG_OUT_FOLDER "ANALOG OUT"
#DEFINE CST_ANALOG_OUT_BASE "OUTPUT "
#DEFINE CST_LEDS_FOLDER "LEDS"
#DEFINE CST_LED_BASE "LED "
#DEFINE CST_DIGITAL_OUT_FOLDER "DIGITAL OUT"
#DEFINE CST_DIGITAL_OUT_BASE "OUTPUT "
VAR_INPUT
   debug : BOOL := True;
END_VAR;
VAR
   cmd : STRING;
   type : STRING;
   output : int;
   
END_VAR;
   cmd := lastCMD.name;
   if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"CHECK BUILT IN : "+cmd);END_IF;
   IF  strLeft(str:=cmd, length:=strLen(str:=CST_BUILTIN_FOLDER) ) <> CST_BUILTIN_FOLDER THEN
      if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"non BUILT IN : "+cmd);END_IF;
      RETURN;
   ELSE
      if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"GOOD BUILT IN : "+cmd);END_IF;
   END_IF;
   cmd := strMid(str:=cmd, start := strLen(str:=CST_BUILTIN_FOLDER)+2,length :=100);
   
   IF strLeft(str:=cmd, length:=strLen(str:=CST_ANALOG_OUT_FOLDER)) = CST_ANALOG_OUT_FOLDER THEN 
      if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"ANALOG : "+cmd);END_IF;
      output := strToInt(str:= strMid(str:=cmd, start:= strLen(str:=CST_ANALOG_OUT_FOLDER+"/"+CST_ANALOG_OUT_BASE)+1, length:=100));
      if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"OUTPUT : "+intToStr(v:=output) + " ou " + strMid(str:=cmd, start:= strLen(str:=CST_ANALOG_OUT_FOLDER+"/"+CST_ANALOG_OUT_BASE)+1, length:=100));END_IF;
      CASE output OF
        1 : AOUT_1 := lastCMD.ival;
        2 : AOUT_2 := lastCMD.ival;
        3 : AOUT_3 := lastCMD.ival;
        4 : AOUT_4 := lastCMD.ival;
      END_CASE;

   END_IF;
   IF strLeft(str:=cmd, length:=strLen(str:=CST_LEDS_FOLDER)) = CST_LEDS_FOLDER THEN 
      if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"LED : "+cmd);END_IF;
      output := strToInt(str:= strMid(str:=cmd, start:= strLen(str:=CST_LEDS_FOLDER+"/"+CST_LED_BASE)+1, length:=100));
      if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"OUTPUT : "+intToStr(v:=output) + " ou " + strMid(str:=cmd, start:= strLen(str:=CST_ANALOG_OUT_FOLDER+"/"+CST_ANALOG_OUT_BASE)+1, length:=100));END_IF;
      CASE output OF
        1 : LED_1 := lastCMD.bval;
        2 : LED_2 := lastCMD.bval;
        3 : LED_3 := lastCMD.bval;
        4 : LED_4 := lastCMD.bval;
      END_CASE;
   END_IF;
   IF strLeft(str:=cmd, length:=strLen(str:=CST_DIGITAL_OUT_FOLDER)) = CST_DIGITAL_OUT_FOLDER THEN 
      if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"DIGITAL = "+cmd);END_IF;
      output := strToInt(str:= strMid(str:=cmd, start:= strLen(str:=CST_DIGITAL_OUT_FOLDER+"/"+CST_DIGITAL_OUT_BASE)+1, length:=100));
      if debug=True THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"OUTPUT : "+intToStr(v:=output) + " ou " + strMid(str:=cmd, start:= strLen(str:=CST_ANALOG_OUT_FOLDER+"/"+CST_ANALOG_OUT_BASE)+1, length:=100));END_IF;
      CASE output OF
        1 : DOUT_1 := lastCMD.bval;
        2 : DOUT_2 := lastCMD.bval;
        3 : DOUT_3 := lastCMD.bval;
        4 : DOUT_4 := lastCMD.bval;
        5 : DOUT_5 := lastCMD.bval;
        6 : DOUT_6 := lastCMD.bval;
        7 : DOUT_7 := lastCMD.bval;
        8 : DOUT_8 := lastCMD.bval;
      END_CASE;
   END_IF;
      // Enable certain options
     
END_FUNCTION;


THREAD_BLOCK thEventMQTT;
VAR
  
  event     : INT := 0  ;
  rc        : INT       ;
END_VAR;
   DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Start MQTT Listener");
   rxd(data := ADDR(payloadReqMQTT), maxsize := SIZEOF(payloadReqMQTT));
   rc := 0;
   WHILE event <> -1 DO
      rc := mqttWaitEvent(timeout := 5);
      IF rc > 0 THEN
         IF CST_DEBUG_MQTT_RECEPT>0 THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Message received");END_IF;
         rxd();
         IF CST_DEBUG_MQTT_RECEPT>0 THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"-Topic= " + rxd.topic); END_IF;
         IF CST_DEBUG_MQTT_RECEPT>0 THEN DebugFmt(message := CST_PREFIX_DEBUG_MQTT+"-Handle= \1", v1 := rxd.handle);END_IF;
         IF CST_DEBUG_MQTT_RECEPT>0 THEN DebugFmt(message := CST_PREFIX_DEBUG_MQTT+"-Size= \1", v1 := rxd.size);END_IF;
         IF CST_DEBUG_MQTT_RECEPT>0 THEN DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"-Message= " + strFromMemory(src := ADDR(payloadReqMQTT), len := rxd.size)); END_IF;
         parseSpBv1CMD(debug:=(CST_DEBUG_MQTT_RECEPT>0));
         checkBuiltIn(debug:=(CST_DEBUG_MQTT_RECEPT>0));
         checkMqttCmd(debug:=(CST_DEBUG_MQTT_RECEPT>0));
         
      END_IF;
      Sleep(delay:=500); 
   END_WHILE;
   DebugMsg(message := CST_PREFIX_DEBUG_MQTT+"Stop MQTT Listener");
END_THREAD_BLOCK;

#END_IF