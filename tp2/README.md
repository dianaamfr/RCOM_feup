# Our notes

# Objetivo aula 2

Na aula 2 pretende-se implementar a fase de estabelecimento do protocolo de ligação de dados. As várias fases podem ser vistas no esquema do slide 14 do pptTabalho1.

Esta fase consiste no envio de uma trama de comando SET (set up); do Emissor para o Recetor.
O Recetor, recebendo por completo a mensagem SET, deve enviar uma trama de resposta UA (unnumbered acknowledgement) ao Emissor para indicar o sucesso da receção da trama.

Posto isto, pretende-se que seja aplicado um timeout de 3s, estabelcendo, assim, o mecanismo de retransmissão do lado do Emissor.
Quando o Receptor não responde (ou o Emissor não recebe uma resposta válida), o Emissor reenvia a trama SET ao fim do intervalo de time-out configurado (por exemplo, 3s) e retransmite no máximo 3 vezes.
Para isso, utilizar-se-a a função alarm (estando um exemplo em alarm.c).

# Especificações uteis

- A trama SET a enviar no Emissor é do tipo unsigned char.
- O Emissor escreve a trama SET na porta de série e espera pelo UA que será eventualmente enviado pelo Recetor se receber corretamente.
- No final desta parte do trabalho, ao implementar o alarme, o Emissor só espera durante um certo tempo (ler acima e nos ppt/guiões).
- SET = FLAG, A, C, BCC, Flag (5 bytes)
- Os valores dos bytes da trama estão declarados em types.c e BCC = A^C

- O Recetor espera pela mensagem SET e, quando esta chega por completo, deve enviar ao Emissor o UA.
- É a máquina de estados, cujos estados estão também definidos em types.c, que vai permitir controlar o estado de receção da mensagem.


