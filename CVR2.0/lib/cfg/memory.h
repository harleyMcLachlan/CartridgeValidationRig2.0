#ifndef _memory_h_
#define _memory_h_

#include <EEPROM.h>

//Setting the eeprom beggining addresses so that each task has it own block of memory 
//Can only store 16 bit values
enum exampleTask_eeprom_enum{
    RIGNUM_EEPROM = 0,
    exampleTask_EEPROM_var1,

};
enum priming_eeprom_enum{
    PRIMING_EEPROM_PWM = 4,
    PRIMING_EEPROM_DOSETIME,
    PRIMING_EEPROM_TIMEOPEN1,
    PRIMING_EEPROM_TIMEOPEN2,
    PRIMING_EEPROM_TIMECLOSE1,
    PRIMING_EEPROM_NUMDOSES,
    PRIMING_EEPROM_ALLOWANCE,
    PRIMING_EEPROM_stagnationTime,
    PRIMING_EEPROM_stagnationTime_2,
    PRIMING_EEPROM_stagnationTime_3,
    PRIMING_EEPROM_BOTTLECOUNTER,
    PRIMING_EEPROM_TIMEBETWEENBATCHES,
    PRIMING_EEPROM_TIMEBETWEENBATCHES_2,
    PRIMING_EEPROM_TIMEBETWEENBATCHES_3,
    PRIMING_EEPROM_NUMBATCHES,
    PRIMING_EEPROM_NUMBOTTLESPERBATCH,
    PRIMING_EEPROM_BATCHCOUNTER,
    PRIMING_EEPROM_CO2ENABLED,
    PRIMING_EEPROM_CONTPRIMENABLED,
    PRIMING_EEPROM_PWMADJUST,
};

enum draining_eeprom_enum{
    draining_EEPROM_drainLength = 50,
};

enum sampling_eeprom_enum{
    sampling_EEPROM_period = 60,
    sampling_EEPROM_length,
    sampling_EEPROM_samplingEnabled,
    sampling_EEPROM_testID,
};

#define system_EEPROM_serialPrintsEnabled 70

void memoryWrite(int address, int number)
{ 
  EEPROM.update(address*2, number >> 8);
  EEPROM.update(address*2 + 1, number & 0xFF);
}

int memoryRead(int address)
{
  return (EEPROM.read(address*2) << 8) + EEPROM.read(address*2 + 1);
}

#endif