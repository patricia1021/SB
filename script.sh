echo TESTE_1
./prog -o "testes/teste1.asm" bin

echo TESTE_2
./prog -o "testes/teste2.asm" bin

echo TESTE_3
./prog -o "testes/teste3.asm" bin

echo TESTE_4
./prog -o "testes/teste4.asm" bin

echo TESTE_5
./prog -o "testes/teste5.asm" bin

echo TESTE_6
./prog -o "testes/teste6.asm" bin

echo TESTE_MOODLE_FAT_A
./prog -o "arquivos_teste_moodle/fat_mod_A.asm" "../simulador/fat_A"

echo TESTE_MOODLE_FAT_B
./prog -o "arquivos_teste_moodle/fat_mod_B.asm" "../simulador/fat_B"

./ligador "../simulador/fat_A.o" "../simulador/fat_B.o" "../simulador/fat.e"
