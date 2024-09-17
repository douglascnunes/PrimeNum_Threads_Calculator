# Processador de números primos usando Threads

O programa desenvolvido em C neste repositório tem como objetivo criar um simulador de pipeline, utilizando lógica de programação paralela, para detectar números primos em uma sequência finita de valores. Ele faz uso de ferramentas de controle de threads, como Mutex, Condicionais e Semáforos.

Há três tipos de threads, cada uma com uma função específica. A thread **geradora** ( :green_square: Generator) tem a função de criar os números a serem analisados, inserindo-os em uma estrutura chamada **PackNumber**. Essa estrutura contém campos necessários para o controle do processamento dos números. As threads **analisadoras** ( :blue_square: Analyzer) recebem os PackNumbers para verificar se o número é primo ou não. Existe apenas uma thread geradora, porém, há **n** threads analisadoras. As threads analisadoras trabalham em conjunto em uma estrutura circular de comunicação, utilizando filas de PackNumbers como buffers. Cada thread analisadora tem um buffer de comunicação de entrada, onde ela pegará novos PackNumbers para analisar, e outro de saída, onde entregará os PackNumbers analisados por ela. Por fim, há uma thread **recebedora** ( :red_square: Receiver), cuja função é receber os PackNumbers com os valores já definidos (se são ou não primos) e exibir a análise no terminal.
<br>
![imagem1_visao_geral](https://github.com/user-attachments/assets/358d2f81-e3e1-47db-8647-4a97f99f9faf)
<br>
## A Proposta
A forma como este problema é estruturado gera diversos desafios no controle de comunicação e no acesso aos PackNumbers nas filas de comunicação e na recepção. Para o desenvolvimento da proposta, o programa deve cumprir os seguintes requisitos:
1. Os PackNumbers, além de armazenarem o número a ser analisado, devem conter as seguintes informações: uma marcação que define se o número é primo ou não, o número de rodadas (voltas) que o PackNumber já realizou no esquema circular de análise, e a identificação de qual thread definiu o número como primo ou divisível.
2. A thread geradora deve, sempre que possível, enviar novos PackNumbers para o **buffer de comunicação de entrada da primeira thread analisadora**.
3. As threads analisadoras devem verificar se o número recebido é primo ou não, dividindo-o pelos PackNumbers presentes em uma fila exclusiva de cada thread (internal buffer). 
   1. O processo deve seguir a seguinte lógica: a thread analisadora retira um PackNumber de seu buffer de comunicação de entrada e verifica se o número é divisível pelo PackNumber na posição correspondente ao número de rodadas do PackNumber em análise. Se o resto da divisão for 0, o número é divisível; se o resto for maior que 0, o número deve continuar sendo avaliado por outras threads analisadoras e, por isso, deve ser enviado para o buffer de comunicação de saída da thread. No entanto, se não houver PackNumbers no buffer interno da thread na posição correspondente ao número de rodadas que o PackNumber já realizou, o número é considerado primo.
   2. PackNumbers contendo números analisados como primos devem ser armazenados na última posição do buffer interno (lembrando que todos os buffers são filas).
   3. Todos os PackNumbers definidos como primos ou divisíveis devem também ser enviados para o buffer da thread recebedora.
4. A thread recebedora deve exibir no terminal o resultados na análise dos PackNumbers em **ordem crescente**.
5. O programa deve acertar 4 argumentos no seu comando de execução: 
   1. **<n>**  números a serem analisados.
   2. **<m>** threads de análise.
   3. **<k>** tamanho limite do buffer de comunicação entre as threads de análise.
   4. **<x>** tamanho do buffer interno de armazenamento de números primos.
6. Caso um número primo seja detectado, mas a quantidade de rodadas do PackNumber desse número primo é superior ao tamanho do buffer interno da thread analisadora, então deve ser detectado que ocorreu um **OVERFLOW**.
   1. A thread analisadora deve indicar no campo de identificação se é primo ou não no PackNumber que o número causou um OVERFLOW e ele deve ser enviado para o buffer da recebedora..
   2. Ao detectar o OVERFLOW, o programa deve encerrar imediatamente, interrompendo qualquer processo que estiver pendente.
7. Se todos os **n** números  foram analisados sem causar OVERFLOW, o programa poderá ser encerrado normalmente.
8. Independente do término do processo de análise das threads analisadoras, a thread recebedora deve exibir o resultado de todos os números analisados antes de encerrar o programa.

<br>
<br>

***
# Configurações necessárias
É necessário realizar alguns passos para que seja possível trabalhar com threads reais em C. A execução do programa desse portfólio foi realizada na IDE Microsoft Visual Studio 2019 (o ano não é importante para o funcionamento) no Windows, com adição de alguns arquivos e configurações.

1. Deve ser adicionado 3 arquivos chamados  **pthread.h**, **sched.h** e **semaphore.h** na pasta **include** no caminho (pode ser qualquer versão anual do Visual Code, no caminho abaixo foi utilizado o 2017 e neste projeto foi usado o 2019):
> C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.11.25503\include
   * Os três arquivos do passo 1 podem ser encontrados em:
[github.com/GerHobbelt/pthread-win32](https://github.com/GerHobbelt/pthread-win32/tree/master)
2. Deve ser adicionado o arquivo **pthreadVC2.lib** na pasta **lib_x86** que se encontra no caminho:
>  C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.11.25503\lib\x86
3. Após a adição dos arquivos no passo dois, deve ser criado um projeto em C no Visual Code e ir em propriedade clicando com o botão direito do mouse sobre o projeto. Configure o projeto para trabalhar com o arquivo **pthreadVC2.lib**.
![sequencia_passos_config_pthreadVC2](https://github.com/user-attachments/assets/cdfed2de-4fbe-4c21-ad1d-d23b4ed5a3b6)
4. Por fim deve ser adicionado o arquivo **pthreadVC2.dll** na pasta **Debug** que fica no mesmo caminho do projeto. Caso não encontre a pasta Debug, execute o projeto para que ela seja criada (será exibido erros, porém pode ignorá-los).
   * Os dois arquivos dos passos 2 e 4 podem ser encontrados em:
[github.com/GerHobbelt/pthread-win32/tree/master/Pre-built.2/lib](https://github.com/Tieske/pthreads-win32/tree/master/Pre-built.2/lib)

***
# Explicando o Código

## Threads Analisadoras
