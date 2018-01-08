
build_kvm ()
{
  ### Build kvm
  g++ -std="c++14" -Wall -ggdb -o kvm kvm.cpp file.cpp
}
clean_kvm ()
{
  rm -f kvm
}

build_guest ()
{
  ### Compile guest
  gcc -fno-stack-protector -c -o guest.o guest.c -m32 -static

  ### Use this guest.elf32 o disassemble using objdump
  ld -o guest.elf32 guest.o -T guest.lds -static -m elf_i386 -M

  ### Use this guest.bin in vmm
  ld -o guest.bin guest.o --oformat binary -T guest.lds -m elf_i386 -M
}
clean_guest ()
{
  rm -f guest.o guest.elf32 guest.bin
}

build ()
{
  build_kvm || return 1
  build_guest || return 1
}
clean ()
{
  clean_kvm
  clean_guest
}
main ()
{
  local cmd="${1}"; shift

  if declare -F "${cmd}" >/dev/null; then
    "${cmd}" "${@}"
  else
    echo "Unknown command \"${cmd}\"" 1>&2
  fi
}

main "${@}"

