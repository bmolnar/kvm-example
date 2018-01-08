ENTRY(main)

SECTIONS {
  binaryimage 0x0000 :
    {
    guest.o
    }
}

