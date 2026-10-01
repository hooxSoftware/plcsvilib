# PlcSvi
SVI library for Bachmann Electronic PLC Developer  

The library creates a task SviUpdate which updates the given
variables in read or write direction. 

This is a really early version and it can only be used by one
PLC-Module. Upcoming version will fix this behaviour.

### Usage

```
VAR_GLOBAL	
	aExternal   : ARRAY[0..43] OF BYTE;
	u32External : UDINT;
END_VAR
```


```
PROGRAM PLC_PRG
VAR
	u32Test : UDINT;
	r32Test : REAL;
	bTest   : BOOL;
	bInit   : BOOL;
	u8Data  : ARRAY[0..43] OF BYTE;
END_VAR

	IF bInit = FALSE THEN
		bInit := TRUE;
		SetTaskTime(100);

		(* SAMPLE/.aExternal *)
		AddVariable('SAMPLE', '.aExternal', ADR(u8Data), sizeof(u8Data);
		
		(* RES/Time_s *)
		AddVariable('RES', 'Time_s', ADR(u32Test), 0);
		
		(* for testing write Time_s in own variable*)
		(* SAMPLE/.u32External *)
		AddVariableWrite('SAMPLE', '.u32External', ADR(u32Test), 0);
	END_IF

;
END_PROGRAM

```

### Elements

#### AddVariable

|Variable|Type|Description|
|---|---|---|
|strModule    |STRING|name of target module|
|strVariable |STRING|name of target variable|
|pData  |DINT|pointer to local variable ADR(...) |
|u32Size |UDINT|size of local variable or 0|

#### AddVariableWrite

|Variable|Type|Description|
|---|---|---|
|strModule    |STRING|name of target module|
|strVariable |STRING|name of target variable|
|pData  |DINT|pointer to local variable ADR(...) |
|u32Size |UDINT|size of local variable or 0|

#### SetTaskTime

|Variable|Type|Description|
|---|---|---|
|u32Time    |UDINT|time in ms 1-1000|