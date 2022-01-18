#include "jswrap_stepcount.h"
#include "stepcount.h"
#include "jsutils.h"

static int _totalSteps = 0;

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "stepCount",
  "generate" : "jswrap_espruino_stepCount",
  "params" : [
    ["x","int","Accelerometer x value"],
    ["y","int","Accelerometer y value"],
    ["z","int","Accelerometer z value"]
  ],
  "return" : ["int","Returns the number of steps counted for this accel interval."]
}
   Access to step counter 
 */
int jswrap_espruino_stepCount(int x, int y, int z) {
    int accMagSquared = (x*x + y*y + z*z)*64;// scale to 8192 = 1g
    int newSteps = stepcount_new(accMagSquared);
    _totalSteps += newSteps;
    return newSteps;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "totalSteps",
  "generate" : "jswrap_espruino_totalSteps",
  "return" : ["int","Returns the number of steps counted since last init."]
}
   Initialise step Counter
 */

int jswrap_espruino_totalSteps(){
    return _totalSteps;
}


/*JSON{
  "type" : "staticmethod",
  "class" : "E",
  "name" : "stepInit",
  "generate" : "jswrap_espruino_stepInit"
}
   Initialise step Counter
 */

void jswrap_espruino_stepInit(){
    stepcount_init();
    _totalSteps = 0;
}

