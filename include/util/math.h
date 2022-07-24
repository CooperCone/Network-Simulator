#pragma once

#define speedOfLight 300000000

#define unitToMili(unit) (1000 * (unit))
#define unitToMicro(unit) (1000 * (unitToMili(unit)))
#define unitToNano(unit) (1000 * (unitToMicro(unit)))

#define kiloToUnit(unit) ((unit) * 1000)
#define megaToUnit(unit) (1000 * (kiloToUnit(unit)))
#define gigaToUnit(unit) (1000 * (megaToUnit(unit)))
