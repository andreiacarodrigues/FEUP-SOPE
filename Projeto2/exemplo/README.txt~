* Projeto realizado pelo grupo 7, turma 5 (T7G05)

O programa foi testado para vários casos:
- gerador começar antes do parque (neste caso os carros vão encontrar o parque fechado)
- gerador acabar depois do encerramento do parque (carros encontram o parque fechado)
- gerador começar depois do parque abrir
- gerador acabar antes do encerramento do parque (deixam de ser gerados mais carros e parque espera pela saída dos carros já estacionados)
- gerador sozinho (são gerados carros que encontram o parque encerrado)
- parque sozinho (parque fica á espera por algum carro durante os segundos que definirmos na sua chamada, encerrando quando o tempo chegar ao limite)

É de salientar que com ticks de relogio baixos enviados ao gerador, por vezes as fifos não trabalham como esperado devido ao pequeno tempo entre a geração dos carros, pelo que facilmente atingem o numero maximo de fifos que podem ser criadas.
É também importante referir que a invocação do parque e do gerador deverá ser feita dentro da pasta 'bin', sendo que é aí que vão ficar os executáveis de ambos aquando da chamada 'make', na pasta do projeto, juntamente com 3 ficheiros : gerador.log, parque.log e estatisticas.txt

O caso que enviamos na pasta 'exemplo' é das seguintes chamadas a partir do terminal:
./parque 5 10
./gerador 20 500000
Neste caso o gerador foi chamado pouco antes da chamada do parque, e também termina depois do parque encerrar.
