unsigned char infun();
void outfun(unsigned char byte);
int func1(int a, int b);
int func2(int a, int b);

int main()
{
  for (;;) {
    func1(0, 1);
  }
}

int func2(int a, int b)
{
  return a + b;
}

int func1(int a, int b)
{
  unsigned char c;
  for (;;) {
    a = func2(a, b);
    if (a > 1000) {
      c = infun();
      if (c == 42) {
        outfun(42);
        asm("hlt");
      }
      c++;
      outfun(c);
    }
    if (a > 2000) {
      asm("hlt");
    }
  }
}


/*
 * I/O routines for communicating with hypervisor
 */
/*
 * insb target it ES:(E)DI
 * lodsb DS:(E)SI into AL
 */
unsigned char infun()
{
  unsigned char byte;
  asm("push %eax");
  asm("insb");
  asm("dec %edi");
  asm("mov 0x0(%edi), %al");
  asm("movb %%al, %0" : "=m"(byte));
  asm("pop %eax");
  return byte;
}
/*
 * stosb AL in ES:(E)DI
 * outsb source is DS:(E)SI
 */
void outfun(unsigned char byte)
{
  asm("push %eax");
  asm("movb %0, %%al" : : "m"(byte));
  asm("mov  %al, 0x0(%esi)");
  asm("outsb");
  asm("dec %esi");
  asm("pop %eax");
}

