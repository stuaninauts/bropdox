
## Roadmap:

1. Criação da estrutura básica do servidor e cliente  
    - Esqueleto do cliente (main, parser de argumentos, loop de comandos)  
    - Servidor aceita conexões e cria thread/processo para cada cliente  

2. Implementar comunicação TCP básica  
    - Criar `connect()` no cliente e `accept()` no servidor  
    - Definir estrutura `packet` para comunicação  
    - Testar envio de comandos simples  

3. Implementar upload/download de arquivos  
    - Criar comandos `upload`, `download`  
    - Fragmentar arquivos em blocos, enviar/receber  
    - Salvar no `sync_dir_<username>` do servidor  

4. Criar sync_dir no cliente  
    - Comando `get_sync_dir`  
    - Criar diretório local se não existir  

5. Implementar inotify no cliente  
    - Monitorar eventos como `IN_CLOSE_WRITE`, `IN_DELETE`, etc.  
    - Enviar alterações automaticamente ao servidor  

6. Sincronizar múltiplos dispositivos  
    - No servidor, propagar mudanças de um dispositivo para os demais  
    - Garantir consistência com `mutex`/`semaphore`  

7. Persistência de dados  
    - Salvar arquivos em disco por usuário (pasta `sync_dir_<username>`)  
    - Ao reiniciar o servidor, restaurar estrutura  

8. Implementar demais comandos (list, delete, etc)  
    - `list_server`, `list_client` com MAC times (usar `stat`)  
    - `delete` remove arquivos sincronizados  
    - `exit` encerra sessão  

## Estrutura de arquivos

myDropbox/
├── client/
│   ├── main.c                     # Início do cliente, parsing de argumentos
│   ├── client.c                   # Lógica principal de comunicação cliente-servidor
│   ├── client.h
│   ├── sync_thread.c              # Thread que monitora o sync_dir (inotify)
│   ├── sync_thread.h
│   ├── file_utils.c               # Funções para manipulação de arquivos
│   ├── file_utils.h
│   ├── network.c                  # Envia/recebe dados via TCP
│   ├── network.h
│   └── protocol.h                 # Definição de estruturas (packets, comandos)
│
├── server/
│   ├── main.c                     # Início do servidor, aceita conexões
│   ├── server.c                   # Lógica principal do servidor
│   ├── server.h
│   ├── client_handler.c           # Funções que tratam comandos de clientes
│   ├── client_handler.h
│   ├── sync_manager.c             # Gerencia propagação de arquivos entre dispositivos
│   ├── sync_manager.h
│   ├── file_utils.c               # Manipula arquivos no servidor
│   ├── file_utils.h
│   ├── network.c                  # Recebe e envia dados via TCP
│   ├── network.h
│   └── protocol.h                 # Mesmo protocolo do cliente
│
├── shared/
│   ├── protocol.h                 # Comandos, structs, tamanhos máximos etc.
│   └── utils.c                    # Funções auxiliares comuns
│   └── utils.h
│
├── Makefile                      # Compila cliente e servidor
└── README.md                     # Explicações de uso, como compilar, etc.

## Detalhamento de arquivos 
| Arquivo                  | Responsabilidade Principal                                                                 |
|--------------------------|--------------------------------------------------------------------------------------------|
| `client/main.c`          | Inicializa o cliente, faz parsing de argumentos da linha de comando e inicia os módulos.   |
| `client/client.c`        | Controla o fluxo principal da aplicação cliente: login, comandos do usuário, sincronização. |
| `client/sync_thread.c`   | Cria uma thread que monitora o diretório `sync_dir` com inotify e envia eventos ao servidor.|
| `client/file_utils.c`    | Lida com leitura, escrita, criação, cópia e comparação de arquivos locais.                 |
| `client/network.c`       | Abstrai a comunicação TCP com o servidor: conexão, envio e recebimento de dados.           |
| `client/protocol.h`      | Define as estruturas dos pacotes (e.g., structs de upload/download), comandos e constantes.|
| `client/*.h` (headers)   | Declaração das outras funções utilizadas pelos respectivos `.c`.                           |
|--------------------------|--------------------------------------------------------------------------------------------|
| `server/main.c`          | Inicia o servidor, abre socket, escuta conexões e cria threads para cada cliente.          |
| `server/server.c`        | Implementa o loop principal do servidor, gerencia autenticação e coordena handlers.        |
| `server/client_handler.c`| Trata requisições individuais de clientes (upload/download/list/delete etc).               |
| `server/sync_manager.c`  | Garante sincronização dos arquivos entre dispositivos do mesmo usuário.                    |
| `server/file_utils.c`    | Gerencia arquivos e diretórios no lado do servidor (e.g., salvar upload, deletar arquivos).|
| `server/network.c`       | Abstrai envio/recebimento de pacotes com clientes via TCP.                                 |
| `server/protocol.h`      | Mesmas definições do cliente, compartilhadas.                                              |
| `server/*.h` (headers)   | Declaração das outras funções utilizadas pelos respectivos `.c`.                           |
|--------------------------|--------------------------------------------------------------------------------------------|
| `shared/protocol.h`             | Definições de comandos, tamanhos máximos, structs padronizadas de comunicação.      |
| `shared/utils.c`                | Funções auxiliares diversas: manipulação de strings, caminhos, logs, tempo etc.     |
| `shared/utils.h`                | Declarações para `utils.c`.                                                         |
