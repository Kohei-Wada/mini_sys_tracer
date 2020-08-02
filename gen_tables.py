#!/usr/bin/python3
import re


def get_syscall_nums(unistd_h):
    syscalls = {}

    for line in open(unistd_h):
         m = re.search(r"^#define\s*__NR_(\w+)\s*(\d+)", line)
         if m:
             (name, number) = m.groups()
             number = int(number)
             syscalls[number] = name

    return syscalls



def write_output(syscalls_h, numbers):
    max_syscall_num = max(numbers.keys())
    with open(syscalls_h, "w") as f:
        print(f"#define MAX_SYSCALL_NUM {max_syscall_num}\n", file=f)
        f.write("struct syscall_entry syscalls[] = \n{\n")

        for i in range(max_syscall_num):
            f.write("    {")

            print(f"{i}, ", end ="", file=f)
            f.write('"')

            try:
                print(f"{numbers[i]}", end="", file=f)
            except:
                f.write("unknown")

            f.write('"')
            f.write("},\n")

        f.write("};")




def main():

    unistd_h = "/usr/include/x86_64-linux-gnu/asm/unistd_64.h"
    syscall_nums = get_syscall_nums(unistd_h)
    write_output("syscallents.h", syscall_nums)



if __name__ == "__main__":
    main()
