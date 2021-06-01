#include <time.h>
#include <stdio.h>
#include "types.hpp"
#include "core.hpp"
#include "logs.hpp"


int main() {
  // Seed RNG
  srand((uint32)time(NULL));

  //
  //                                    ___
  //                                ,-""   `.
  //                              ,'  _   e )`-._
  //                             /  ,' `-._<.===-'
  //                            /  /
  //                           /  ;
  //               _.--.__    /   ;
  //  (`._    _.-""       "--'    |
  //  <_  `-""                     \
  //   <`-                          :
  //    (__   <__.                  ;
  //      `-.   '-.__.      _.'    /
  //         \      `-.__,-'    _,'
  //          `._    ,    /__,-'
  //             ""._\__,'< <____
  //                  | |  `----.`.
  //                  | |        \ `.
  //                  ; |___      \-``
  //                  \   --<
  //                   `.`.<
  //                     `-'
  int status = core::run();
  if (status != 0) {
    char discarded_input[256];
    logs::info("Exited with status %d", status);
    logs::info("Please read log and press enter to exit");
    scanf("%s", discarded_input);
  }
}
