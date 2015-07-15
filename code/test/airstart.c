/* airstart.c
 */

#include "syscall.h"


int main() {

	Exec("../test/airportsim", sizeof("../test/airportsim"));
	Exec("../test/airportsim", sizeof("../test/airportsim"));

}
