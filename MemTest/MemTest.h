/****************************************************************************
 * MemTest - Free Memory Checker
 *
 * Initialize with begin() and call check() to see if Free Memory has dropped.
 *
 * Methods:
 *   begin()
 *   check()
 *   hits()
 *   freeMemoryMin()
 *   freeMemory() (static)
 ****************************************************************************/

#ifndef FREE_MEM_H
#define FREE_MEM_H

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

class MemTest {
  int loop_counter = -1;
  int free_mem_hits = -1;
  int free_mem, free_mem_min;
  int modulus, delay_on_hit_ms;
  Stream* ostream;

public:

  /*
   * begin() - initial free memory checker
   *   ios - Serial typically (caller initializes ( .begin(bps) ))
   *   _modulus - how often to actually read and check free memory
   *   _delay_on_hit_ms - how long to delay while observer reads the report
   */
  void begin(Stream& ios, int _modulus=1, int _delay_on_hit_ms=0) {
    ostream = &ios;
    modulus = _modulus;
    delay_on_hit_ms = _delay_on_hit_ms;
    free_mem_min = freeMemory();
  }

  /*
   * check() - check free memory
   *   Called from loop() typically.
   *   Will normally just check periodically if modulus is set in begin().
   * returns true on free memory decrease
   *   Note: hit:0 is reported after the completion of the caller's setup() and the first loop() functions.
   *         It's not an error but a report of the baseline after all objects are constructed and initialized.
   */
  bool check(void) {
    loop_counter++;
    if (modulus == 1 or loop_counter % modulus == 0) {
      free_mem = freeMemory();
      if (free_mem < free_mem_min) {
        free_mem_hits++;
        ostream->print("Free memory:");
        ostream->print(free_mem);
        ostream->print(" hit:");
        ostream->print(free_mem_hits);
        ostream->println(" ***********************************");
        free_mem_min = free_mem;
        if (delay_on_hit_ms) delay(delay_on_hit_ms);
        return true;
      }
    }
    return false;
  } // check()

  /*
   * hits() - get number of free memory hits
   *
   * returns the number of lower free memory reads.
   *   If the console is busy you may want to call this periodically
   *   to see if there have been new memory events that weren't captured.
   *   Call freeMemory() and freeMemoryMin() to get the current stats.
   */
  int hits(void) {
    return free_mem_hits;
  }

  /*
   * freeMemoryMin() - call to get the lowest free memory value captured.
   *
   * returns free_mem_min which tracks the low point.
   */
  int freeMemoryMin(void) {
    return free_mem_min;
  }

  /*
   * freeMemory() - read free memory
   *   courtesy of Adafruit
   *   public and static for your convenience
   */
  static
  int freeMemory(void) {
    char top;
  #ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
  #else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
  #endif  // __arm__
  } // freeMemory()

}; // MemTest class

#endif // _H
