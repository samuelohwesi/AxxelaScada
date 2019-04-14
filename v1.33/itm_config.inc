
#IFDEF NOT __INC_ITM_CONFIG__ THEN
#DEFINE __INC_ITM_CONFIG__
INCLUDE rtcu.inc
INCLUDE math.inc

#DEFINE CST_MEMORY_INDEX_CONFIG        99
#DEFINE CST_MEMORY_INDEX_PARAM         100
#DEFINE CST_MAX_SCANCLASS              8


#DEFINE CST_SCANCLASS_30_S      0
#DEFINE CST_SCANCLASS_1_MN      1
#DEFINE CST_SCANCLASS_10_MN      2
#DEFINE CST_SCANCLASS_30_MN      3
#DEFINE CST_SCANCLASS_1_HR      4
#DEFINE CST_SCANCLASS_6_HR      5
#DEFINE CST_SCANCLASS_12_HR     6
#DEFINE CST_SCANCLASS_1_DAY      7
#DEFINE CST_SCANCLASS_7_DAY      8


STRUCT_BLOCK ALIGN typeCfg
   date              : ARRAY[0..18] OF SINT;// 2016-05-10 10:50 STRING;
   name              : ARRAY[0..20] OF SINT;//  STRING;
   timeZone          : SINT := 1; // TimeZone Nigeria
   ScanClass          : ARRAY[0..CST_MAX_SCANCLASS] OF DINT;// Scan RAte
END_STRUCT_BLOCK;

STRUCT_BLOCK ALIGN typePrm
  val                :DINT;
END_STRUCT_BLOCK;


VAR
   cfg : typeCfg;
   prm : typePrm;
   
END_VAR;

FUNCTION displayConfig : INT;
VAR
      i : int;
END_VAR;
   DebugFmt(message:="Config : ");
   DebugFmt(message:="  -> Date = "+strFromMemory(src:=ADDR(cfg.date), len:=SIZEOF(cfg.date)));
   DebugFmt(message:="  -> Name = "+strFromMemory(src:=ADDR(cfg.name), len:=SIZEOF(cfg.name)));
   FOR i:=0 TO CST_MAX_SCANCLASS DO
      DebugFmt(message:="  -> ScanClass[\1] = \4",v1 := i,v4:=cfg.ScanClass[i]);
   END_FOR;
END_FUNCTION; 

FUNCTION setDefaultConfig : INT;
   strToMemory(dst:=ADDR(cfg.date), str:="2018-01-01 00:00", len:=strLen(str:="2018-01-01 00:00"));
   strToMemory(dst:=ADDR(cfg.name), str:="uServe", len:=strLen(str:="uServe"));
   cfg.ScanClass[CST_SCANCLASS_30_S] :=                30; // 30s
   cfg.ScanClass[CST_SCANCLASS_1_MN] :=                60; // 1 Minute
   cfg.ScanClass[CST_SCANCLASS_10_MN] :=          10 * 60; // 10 Minutes
   cfg.ScanClass[CST_SCANCLASS_30_MN] :=          30 * 60; // 30 Minutes
   cfg.ScanClass[CST_SCANCLASS_1_HR] :=           60 * 60; // 1 Hour
   cfg.ScanClass[CST_SCANCLASS_6_HR] :=       6 * 60 * 60; // 6 Hours
   cfg.ScanClass[CST_SCANCLASS_12_HR] :=     12 * 60 * 60; // 12 Hours
   cfg.ScanClass[CST_SCANCLASS_1_DAY] :=     24 * 60 * 60; // 1 Day
   cfg.ScanClass[CST_SCANCLASS_7_DAY] := 7 * 24 * 60 * 60; // 7 Days
END_FUNCTION;

FUNCTION setConfigToFRAM : INT;
VAR_INPUT
   debug    : bool := false;
END_VAR;
   SaveDataF(index:=CST_MEMORY_INDEX_CONFIG, data:=ADDR(cfg) , datasize := Sizeof(typeCfg));
   IF debug THEN
      DebugFmt(message:="setConfigToFRAM SaveDataF \1 : (\2)",v1:= CST_MEMORY_INDEX_CONFIG,v2:= SizeOf(typeCfg));
      DebugFmt(message:="  -> cfg.date[0] = \1 : ",v1:= int(cfg.date[0]));
   END_IF;
END_FUNCTION; 

FUNCTION getConfigFromFRAM : INT;
VAR_INPUT
   debug    : bool := false;
END_VAR;
   LoadDataF(index:=CST_MEMORY_INDEX_CONFIG, data:=ADDR(cfg) , datasize := Sizeof(typeCfg));
   if cfg.date[0]<>16#32 and cfg.date[1]<>16#30 THEN
      setDefaultConfig();
      setConfigToFRAM();
   end_if;
   IF debug THEN
      DebugFmt(message:="getConfigFromFRAM LoadDataF \1 : (\2)",v1:= CST_MEMORY_INDEX_CONFIG,v2:= SizeOf(typeCfg));
      displayConfig();
   END_IF;
   
END_FUNCTION;



FUNCTION getParamFromFRAM : INT;
VAR_INPUT
   debug    : bool := false;
END_VAR;
   LoadDataF(index:=CST_MEMORY_INDEX_PARAM, data:=ADDR(prm) , datasize := Sizeof(typePrm));
   IF debug THEN DebugFmt(message:="getParamFromFRAM LoadDataF \1 : (\2)",v1:= CST_MEMORY_INDEX_PARAM,v2:= SizeOf(typePrm)); END_IF;
END_FUNCTION;

FUNCTION setParamToFRAM : INT;
VAR_INPUT
   debug    : bool := false;
END_VAR;
   SaveDataF(index:=CST_MEMORY_INDEX_PARAM, data:=ADDR(prm) , datasize := Sizeof(typePrm));
   IF debug THEN DebugFmt(message:="setParamToFRAM SaveDataF \1 : (\2)",v1:= CST_MEMORY_INDEX_PARAM,v2:= SizeOf(typePrm)); END_IF;
END_FUNCTION;



#END_IF