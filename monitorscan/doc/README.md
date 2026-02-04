DOCUMENTAÇÃO – MonitorScan

Ass: Nathan Rodrigues de Almeida

Programa em C para Windows que monitora o fluxo de uma determinada pasta, organizando de forma manual ou automática.

Ambiente:

Windows 10/11

Windows Server

Compilador

GCC (MSYS2 / MinGW64)

Pasta Monitorada:

\\10.1.1.180\publico\Fiscal\Scan\


Estrutura do programa

\\10.1.1.180\publico\Fiscal\Scan\

├─ Pasta principal / monitorscan / entrada dos arquivos

├─ A\

├─ B\

├─ ...

├─ Z\

Modo de Operação:

1\.1 Modo Automático

Arquivos são enviados para a pasta correspondente à primeira letra do nome

Exemplo:

Arquivo

danfe011515 --> D\

texto.txt --> T\

1\.2 Modo Manual

Criar regras personalizadas do tipo:

Arquivo começa com → Pasta destino

Exemplo:

danfe011515 --> B\

texto.txt --> T\



Comandos do teclado

O programa aceita comandos sem interromper o monitoramento

| Tecla | Ação                        |

\| ----- | --------------------------- |

| `R`   | Criar nova regra manual     |

| `C`   | Limpar todas as regras      |

| `A`   | Voltar para modo automático |

| `Q`   | Encerrar o programa         |


Monitoramento de Arquivos

Utiliza ReadDirectoryChangesW

Modo OVERLAPPED (assíncrono)

Execução em Outras Máquinas

Para executar em outra máquina:

Copiar apenas:

monitorscan.exe


Colocar em uma pasta local (ex: C:\MonitorScan\)

Garantir acesso ao compartilhamento de rede

Executar normalmente!

Compilação:

gcc main.c -o monitorscan.exe -static



Limitações Conhecidas

O programa precisa permanecer aberto para funcionar

Interface via console

Regras não persistem após reiniciar (por enquanto)






