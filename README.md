### EP1 - MAC0352 - Redes de Computadores e Sistemas Distribuídos

**Professor:** Daniel Batista

**Aluno:** Francisco Eugênio Wernke

**NUSP:** 11221870

O código presente neste arquivo executa um servidor MQTT versão 3.1.1 e aceita conexões de clientes mosquitto\_sub e mosquitto\_pub.

Para compilar o código:

```bash
  make
```

Para executar o servidor em uma porta:

```bash
  ./ep1 <PORTA>
```

Para inscrever um cliente mosquitto (usando a interface mosquitto_sub):

```bash
  mosquitto_sub -V 311 -p <PORTA_DO_SERVIDOR> -h 127.0.0.1 -t <TOPICO>
```

Para publicar uma mensagem com um cliente mosquitto (usando a interface mosquitto_pub):

```bash
  mosquitto_pub -V 311 -p <PORTA_DO_SERVIDOR> -h 127.0.0.1 -t <TOPICO> -m <MENSAGEM>
```
