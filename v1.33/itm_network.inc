//-----------------------------------------------------------------------------
//itm_network.inc, created 2018-07-24
// 
//-----------------------------------------------------------------------------


#IFDEF NOT __INC_ITM_NETWORK__ THEN
#DEFINE __INC_ITM_NETWORK__
INCLUDE rtcu.inc
INCLUDE math.inc
INCLUDE .\itm_util.inc
INCLUDE .\itm_mqtt.inc

#DEFINE CST_PREFIX_DEBUG_NETWORK "<* NET *> "  
   
VAR
   GSM_OFF : BOOL := False;
   TURBO   : BOOL := OFF;
   ENCORE   : BOOL := ON;
   
   netInfo   : ARRAY [1..4] OF netGetInformation;
   oldnetStatus : ARRAY [1..4] OF sint:= 0,0,0,0;
   toBeOpen : ARRAY [1..4] OF BOOL:= True,True,False,True;
   currentIface    : SINT := -1; // Network interface to use: 1 for Mobile, 2 for LAN.
   
   
   gw_media : SINT := 0;

END_VAR;


FUNCTION printNetInfo;
VAR_INPUT
   iface : sint;
END_VAR;
  
  DebugFmt(message:=CST_PREFIX_DEBUG_NETWORK+"Network interface \1:", v1 := iface);
  DebugFmt(message:=CST_PREFIX_DEBUG_NETWORK+" Status \1", v1 := netInfo[iface].status);
  IF netInfo[iface].status > 1 THEN
    DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" Phy addr " + netInfo[iface].phy_addr);
    IF netInfo[iface].dhcp THEN
        DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" Dynamic IP address");
    ELSE
        DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" Static IP address");
    END_IF;
    DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" IP addr " + sockIPToName(ip := netInfo[iface].ip));
    DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" Mask    " + sockIPToName(ip := netInfo[iface].subnetMask));
    DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" Gateway " + sockIPToName(ip := netInfo[iface].gateway));
    IF netInfo[iface].AutoDNS THEN
        DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" Automatic DNS");
    ELSE
        DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" Manual DNS");
    END_IF;
    DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" DNS1    " + sockIPToName(ip := netInfo[iface].DNS1));
    DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+" DNS2    " + sockIPToName(ip := netInfo[iface].DNS2));
  END_IF;
END_FUNCTION;


FUNCTION setGW;
VAR_INPUT
   open : bool;
END_VAR;
VAR
   rc : INT;
END_VAR;
   


   if open THEN
      DebugMsg(message := "Set gateway to Open");
   ELSE
      DebugMsg(message := "Set gateway to Close");
   END_IF;
   if gwEnabled() THEN
      DebugMsg(message := "Device is ready to connect");
   END_IF;
   if gwIsOpen() THEN
      DebugMsg(message := "Device is already connected");
   END_IF;
   IF gw_media <> 3 AND currentIface = 4 THEN
    // Select media
      DebugMsg(message := "Use wifi");
      gwSetMedia(media:=1); // Use ethernet
    // Set curent media
      gw_media := 3;
   ELSIF gw_media <> 1 AND (currentIface = 2 or currentIface = 3) THEN
    // Select media
      DebugMsg(message := "Use ethernet");
      gwSetMedia(media:=1); // Use ethernet
    // Set curent media
      gw_media := 1;
   ELSIF gw_media <> 0 AND currentIface = 1  THEN
    // Select media
    DebugMsg(message := "Use GPRS");
    gwSetMedia(media:=0); // Use GPRS
    // Set curent media
     gw_media := 0;
   END_IF;
   if NOT open and gwIsOpen() THEN
      DebugMsg(message := "Stopping gateway");
      rc := gwClose();
      IF rc < 1 THEN
         DebugFmt(message := "  gwClose=\1", v1 := rc);
      END_IF;
   END_IF;
   IF open and NOT gwIsOpen() THEN
      DebugMsg(message := "Starting gateway");
      rc := gwOpen();
      IF rc < 1 THEN
         DebugFmt(message := "  gwOpen=\1", v1 := rc);
      END_IF;
   END_IF;
END_FUNCTION;


FUNCTION switchToIface;
VAR_INPUT
   iface : sint;
END_VAR;
   DebugFmt(message:=CST_PREFIX_DEBUG_NETWORK+"Switch Network from \1 to \2",v1:=currentIface, v2:=iface );
   //disconnectMQTT();
   currentIface := iface;
   IF currentIface>-1 THEN
      connectMQTT(iface:=currentIface);
      setGW(open:=True);
   END_IF;
END_FUNCTION;


FUNCTION updateNetInfo;
VAR 
      i : SINT;
END_VAR;

   FOR i:=1 to 4 DO
      if toBeOpen[i] THEN
         netInfo[i](iface:=i);
         //DebugFmt(message:=CST_PREFIX_DEBUG_NETWORK+" netInfo iface[\1].status=\2", v1:= i, v2 := netInfo[i].status);
         if netInfo[i].status = 1 and oldnetStatus[i] = 0 then
            oldnetStatus[i] := 1;
            if netOpen(iface:=i)=0 THEN
               netInfo[i](iface:=i);
               oldnetStatus[i] := 2;
            END_IF;
         ELSIF  netInfo[i].status = 3 and oldnetStatus[i] < 3 then
            printNetInfo(iface:=i);
            oldnetStatus[i] := 3;
         ELSIF   netInfo[i].status < 3 and oldnetStatus[i] = 3 then
            DebugFmt(message:=CST_PREFIX_DEBUG_NETWORK+" Loosing connexion iface \1",v1:=i);
            if currentIface=i then  switchToIface(iface:=-1); end_if;
            oldnetStatus[i] := netInfo[i].status;
         END_IF;
     END_IF;
   END_FOR;
END_FUNCTION;


THREAD_BLOCK thManageNetwork;
VAR
   debug :  BOOL := True;
   retVal : BOOL := FALSE;
   rc :     INT := 0;
END_VAR;
   DebugFmt(message:=CST_PREFIX_DEBUG_NETWORK+"Config Network");
   // Open the GPRS connetion
   rc:=gprsOpen();
   DebugFmt(message:=CST_PREFIX_DEBUG_NETWORK+"gprsOpen()=\1",v1:=rc);
 
   // Wait for GPRS connected (after this, we are connected to the Internet)
   WHILE NOT gprsConnected() DO
      DebugMsg(message:=CST_PREFIX_DEBUG_NETWORK+"Waiting for GPRS connection");
      Sleep(delay:=2500);
   END_WHILE;

   WHILE 1 <> -1 DO
      //DebugFmt(message:=CST_PREFIX_DEBUG_NETWORK+"currentIface := \1",v1:=currentIface);
      updateNetInfo();
//      if currentIface<>2 or not netConnected(iface:=2)THEN
//         if netConnected(iface:=2) THEN
//            switchToIface(iface:=2);
//         elsif currentIface<>4 AND netConnected(iface:=4) THEN
//            switchToIface(iface:=4);
//         elsif currentIface<>4 AND currentIface<>1 AND netConnected(iface:=1) THEN
         if  currentIface<>1 AND netConnected(iface:=1) THEN
            switchToIface(iface:=1);
         end_if;
               
     // end_if;
     
      Sleep(delay :=10000);
      
   END_WHILE;
   DebugMsg(message := "Network check");
END_THREAD_BLOCK;
#END_IF;
 