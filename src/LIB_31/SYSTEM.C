/*
 *  Released under the GNU LGPL.  See http://www.gnu.org/licenses/lgpl.txt
 *
 *  This program is part of the DeSmet C Compiler
 *
 *  This library is free software * you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundatation * either version 2.1 of the License, or
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY * without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 *  License for more details.
 */
/*
**	int system(char cmd[]);
**
**	invoke command.com
*/

int system(char cmd[]) {
	char path[65], arg[129];

	if(getenv("COMSPEC", path, sizeof(path))) {
		arg[0] = '/';
		arg[1] = 'c';
		strcpy(&arg[2], cmd);
		if(!exec(path, arg)) return 0;
		}
	return -1;
	}
