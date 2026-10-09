================================================================================
  S.A.R.A.  -  Sistema de Analise e Rastreamento de Anomalias
================================================================================

  Vigilância epidemiológica inteligente.
  Detecção de anomalias estatísticas antes que se tornem crises.

  Versão:  7.0
  Lingua:  C++17
  Autor:   Caick Jose Correia dos Santos
  Ano:     2026

--------------------------------------------------------------------------------
  SOBRE
--------------------------------------------------------------------------------

  O S.A.R.A. e um sistema em C++ que monitora a incidência de patologias
  por bairro e identifica anomalias estatísticas - desvios do padrão
  esperado de casos - antes que se consolidem em crises no sistema de saúde.

  O sistema aplica métricas do domínio de saúde publica:

    - Incidência por 100 mil habitantes
    - Media móvel de 4 semanas
    - Taxa de crescimento percentual
    - Tendencia direcional (subindo, caindo, estável)

  Cada bairro e automaticamente classificado em quatro faixas de risco
  adaptadas dos protocolos da OMS.

--------------------------------------------------------------------------------
  FUNCIONALIDADES
--------------------------------------------------------------------------------

  - Cadastro de bairros com população e patologias dinâmicas
  - Serie temporal semanal com histórico completo por bairro x patologia
  - Calculo automático de incidência, media móvel e taxa de crescimento
  - Classificação de risco: Baixo, Medio, Alto e Critico
  - Gráfico ASCII de serie temporal renderizado no terminal
  - Persistência automática em sara.db com backup em sara.db.bak
  - Importação em lote via CSV com tratamento de exceções
  - Exportação em quatro formatos: CSV, grafo Mermaid, JSON e relatório
  - Modo batch via linha de comando para automação

--------------------------------------------------------------------------------
  COMO COMPILAR
--------------------------------------------------------------------------------

  Abra o arquivo "Projeto SARA.sln" no Visual Studio 2022.

  Em seguida:
    1. Selecione o modo Release na barra superior
    2. Pressione F7 (Build > Rebuild Solution)
    3. Aguarde a mensagem "Build succeeded"

  O executável sera gerado em:
    x64\Release\Projeto SARA.exe

  Para rodar:
    - Pelo Visual Studio: pressione Ctrl + F5
    - Pelo Windows: navegue ate a pasta x64\Release e execute o .exe
--------------------------------------------------------------------------------
  COMO USAR
--------------------------------------------------------------------------------

  Modo interativo
  ---------------
    Projeto_SARA.exe

  Menu de opções:

    1. Cadastrar bairro
    2. Registrar casos (patologia + semana)
    3. Ver ranking de risco
    4. Exportar relatórios
    5. Remover bairro
    6. Salvar estado agora
    7. Gráfico de serie temporal (ASCII)
    8. Importar CSV em batch
    0. Sair (salva automaticamente)

  Modo batch
  ----------
    Projeto_SARA.exe --batch    carrega, exporta tudo e sai
    Projeto_SARA.exe --help     exibe ajuda

  Formato do CSV de importação
  ----------------------------
    # Linhas com # são ignoradas
    Benedito Bentes,110746
    Benedito Bentes,Dengue,2026-W39,60
    Ponta Verde,Dengue,2026-W39,10

    - 2 campos: cadastra bairro (nome, população)
    - 4 campos: registra caso (bairro, patologia, semana, casos)

--------------------------------------------------------------------------------
  ARQUIVOS GERADOS
--------------------------------------------------------------------------------

    sara.db                       Estado persistente (auto-salvo ao sair)
    sara.db.bak                   Backup automático do estado anterior
    SARA_Relatorio.csv            Planilha tabular
    SARA_Grafo.md                 Grafo Mermaid colorido por risco
    SARA_Dados.json               Estruturado para integração
    SARA_Relatorio_Executivo.txt  Relatório para tomada de decisão

--------------------------------------------------------------------------------
  METRICAS DE REFERENCIA
--------------------------------------------------------------------------------

  Incidência por 100 mil habitantes
  ---------------------------------
    incidência = (casos ativos / população) * 100.000

  Faixas de classificação de risco
  --------------------------------
    A OMS define apenas o limiar epidemiológico (>= 300 casos/100k).
    As faixas intermediarias são adaptações didáticas para este projeto.

    Baixo     incidência inferior a 50
    Medio     incidência entre 50 e 99
    Alto      incidência entre 100 e 299
    Critico   incidência igual ou superior a 300

--------------------------------------------------------------------------------
  PARADIGMAS APLICADOS
--------------------------------------------------------------------------------

    Orientado a objetos   Classes com encapsulamento e invariantes internas
    Imperativo            Menus, controle de fluxo, persistência em disco
    Funcional             std::accumulate, std::count_if, std::transform,
                          funções puras passadas como valor

  Estruturas de controle
  ----------------------
    if / else             Validação de entrada, classificação de risco
    switch / case         Rótulos de risco, menus de navegação
    while                 Leitura validada, carregamento de arquivo
    do while              Menu principal, confirmação de exportação
    for (range-for)       Iterações sobre coleções

--------------------------------------------------------------------------------
  ESTRUTURA DO CODIGO
--------------------------------------------------------------------------------

    Projeto SARA.cpp

      enum class Risco

      namespace anônimo (parte 1)   - utilitários e helpers
        - leitura validada (lerInt, lerLinha)
        - parse seguro (stoiSeguro)
        - helpers (slugify, semanaAtual, classificarRisco)
        - rótulos de risco (riscoLabel, riscoIcone)

      class RegistroDoenca          - serie temporal de uma patologia
      class Bairro                  - população + patologias + métricas
      class SARA                    - coleção + persistência + import

      namespace anônimo (parte 2)   - funções puras, exportadores, menus
        - funções puras (ehRiscoCritico, ehRiscoAlto)
        - exportadores (CSV, Mermaid, JSON, executivo)
        - menus interativos

      int main()                    - modo batch + loop principal

--------------------------------------------------------------------------------
  DADOS DE EXEMPLO
--------------------------------------------------------------------------------

  Populações conforme Censo 2022 (IBGE) para Maceio:

    Bairro                     População
    Benedito Bentes            110.746
    Tabuleiro do Martins        61.194
    Ponta Verde                 28.591
    Patuscara                     5.500
    Centro                       3.200

  Casos registrados nas semanas epidemiológicas 2026-W35 a 2026-W39
  para Dengue, Zika, Chikungunya e Covid-19.

  Referencias:
    ONU 2026..............  Brasil: 214.211.951 habitantes
    IBGE Censo 2022.......  Maceio: dados por bairro
    OMS...................  Limiar epidemiológico: >= 300 casos / 100 mil hab.
    Ministério da Saúde...  Informe DAS Alagoas 2026

--------------------------------------------------------------------------------
  AUTOR
--------------------------------------------------------------------------------

    Caick Jose Correia dos Santos
    Projeto acadêmico - 2026

================================================================================
  FIM
================================================================================