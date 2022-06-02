FILE = "./src/masks.h"

def single_bitboard():
    with open(FILE, "a") as f:
        f.write("const U64 single_bitboard[64] = {\n")
        for i in range(64):
            f.write(f"    {2**i}ULL")
            if i != 63:
                f.write(",\n")

        f.write("\n};\n")
        f.close()

if __name__ == '__main__':
    #Clear the file
    with open(FILE, "w") as f:
        f.truncate(0)
        f.close()

    with open(FILE, "a") as f:
        f.write("#pragma once \n")
        f.write("typedef unsigned long long U64;\n\n")
        f.close()

    single_bitboard()
    print("Finished")