<h1 align="center" font-size="200em"><b>Simulador da Arquitetura de Von Neumann e Pipeline MIPS</b></h1>

<div align = "center" >
<!-- imagem -->

[![requirement](https://img.shields.io/badge/IDE-Visual%20Studio%20Code-informational)](https://code.visualstudio.com/docs/?dv=linux64_deb)
![Make](https://img.shields.io/badge/Compilacao-Make-orange)
![Linguagem](https://img.shields.io/badge/Linguagem-C%2B%2B-blue)
</div>

## ❗ Detalhes do programa
Após o carregamento dos processos, o quantum é forçadamente colocado com um valor baixo, para que a execução seja interrompida. Após dita interrupção, o processo volta para o final da fila e tem seu quantum recalculado para a faixa de valores padrão (20 a 50ms). 

### Modelo do input
* Salvar na memória
  * = | Registrador de destino | Valor
  * = 0 150
* Adição, subtração, divisão, multiplicação
  * Operação (+, -, /, *) | Registrador de destino | Registrador com o primeiro número | Registrador com o segundo número
  * / 3 0 1
* Loop
  * @ | Registrador inicial/destino | Registrador final | Número de iterações
  * @ 0 3 10
  * Somar conteúdos dos registradores 0 a 3 em 10 iterações, com resultado salvo no registrador 0.
* Condicional
  * ? | Primeiro valor | Segundo valor | (<, >, =, !)
  * ? 7 4 >
* Ler da memória
  * & | Registrador de leitura
  * & 0
* Uso de periféricos
  * $ | Periférico desejado
  * $ 1

## 👾Compilação e execução
* Especificações da máquina em que o código foi rodado:
  * Processador Intel Core i7, 12th Gen;
  * Sistema Operacional Ubuntu 22.04.5;
  * 16GB de RAM.
* | Comando                |  Função                                                                                           |                     
  | -----------------------| ------------------------------------------------------------------------------------------------- |
  |  `make clean`          | Apaga a última compilação realizada contida na pasta build                                        |
  |  `make`                | Executa a compilação do programa utilizando o gcc, e o resultado vai para a pasta build           |
  |  `make run`            | Executa o programa da pasta build após a realização da compilação                                 |


## Contato

<div>
 <br><p align="justify"> Julia Rezende</p>
 <a href="https://t.me/juliarezende34">
 <img align="center" src="https://img.shields.io/badge/Telegram-2CA5E0?style=for-the-badge&logo=telegram&logoColor=white"/> 
 </div>
<a style="color:black" href="mailto:juliarezende34@gmail.com?subject=[GitHub]%20Source%20Dynamic%20Lists">
✉️ <i>juliarezende34@gmail.com</i>
</a>

<div>
 <br><p align="justify"> Mariana Rodrigues Lamounier Melo</p>
 <a href="https://t.me/MariRodriguess0">
 <img align="center" src="https://img.shields.io/badge/Telegram-2CA5E0?style=for-the-badge&logo=telegram&logoColor=white"/> 
 </div>
<a style="color:black" href="mailto:mariana.itapec@gmail.com?subject=[GitHub]%20Source%20Dynamic%20Lists">
✉️ <i>mariana.itapec@gmail.com</i>
</a>
