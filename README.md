# S.A.R.A. — Sistema de Análise e Rastreamento de Anomalias

Vigilância epidemiológica inteligente em terminal. Detecção de anomalias estatísticas antes que se tornem crises.

**Versão:** 4.0.0 · **Linguagem:** C++17 · **Ano:** 2026

---

## Sobre

O S.A.R.A. é um sistema de linha de comando que monitora a incidência de patologias por bairro e identifica anomalias estatísticas — desvios do padrão esperado de casos — antes que se consolidem em crises no sistema de saúde.

O sistema aplica métricas do domínio de saúde pública:

- Incidência por 100 mil habitantes
- Média móvel de 4 semanas
- Taxa de crescimento percentual
- Tendência direcional (subindo, caindo, estável)

Cada bairro é automaticamente classificado em quatro faixas de risco adaptadas dos protocolos da OMS.

---

## Funcionalidades

- Cadastro de bairros com população e patologias dinâmicas
- Série temporal semanal com histórico completo por bairro × patologia
- Cálculo automático de incidência, média móvel e taxa de crescimento
- Classificação de risco em quatro faixas: **Baixo**, **Médio**, **Alto**, **Crítico**
- Gráfico ASCII de série temporal renderizado no terminal
- Persistência automática em `sara.db` com backup em `sara.db.bak`
- Importação em lote via CSV com tratamento de exceções
- Exportação em quatro formatos: CSV, Mermaid, JSON e relatório executivo
- Modo batch via linha de comando para automação

---

## Como compilar

1. Abra o arquivo `Projeto SARA.sln` no **Visual Studio 2022**.
2. Selecione o modo **Release** na barra superior.
3. Pressione **F7** (Build → Rebuild Solution).
4. Aguarde a mensagem `Build succeeded`.

O executável será gerado em `x64/Release/Projeto SARA.exe`.

---

## Como usar

### Modo interativo

```
Projeto_SARA.exe
```

Menu de opções:

| Opção | Ação |
|:-----:|:-----|
| 1 | Cadastrar bairro |
| 2 | Registrar casos (patologia + semana) |
| 3 | Ver ranking de risco |
| 4 | Exportar relatórios |
| 5 | Remover bairro |
| 6 | Salvar estado agora |
| 7 | Gráfico de série temporal (ASCII) |
| 8 | Importar CSV em batch |
| 0 | Sair (salva automaticamente) |

### Modo batch

```
Projeto_SARA.exe --batch     carrega, exporta tudo e sai
Projeto_SARA.exe --help      exibe ajuda
```

### Formato do CSV de importação

Linhas com `#` são ignoradas.

```
Benedito Bentes,110746
Benedito Bentes,Dengue,2026-W39,60
Ponta Verde,Dengue,2026-W39,10
```

- **2 campos:** cadastra bairro (nome, população)
- **4 campos:** registra caso (bairro, patologia, semana, casos)

---

## Arquivos gerados

| Arquivo | Descrição |
|:--------|:----------|
| `sara.db` | Estado persistente (auto-salvo ao sair) |
| `sara.db.bak` | Backup automático do estado anterior |
| `SARA_Relatorio.csv` | Planilha tabular |
| `SARA_Grafo.md` | Grafo Mermaid colorido por risco |
| `SARA_Dados.json` | Estruturado para integração |
| `SARA_Relatorio_Executivo.txt` | Relatório para tomada de decisão |

---

## Métricas de referência

### Incidência por 100 mil habitantes

```
incidência = (casos atuais / população) × 100.000
```

### Faixas de classificação de risco

A OMS define apenas o limiar epidemiológico (≥ 300 casos / 100k). As faixas intermediárias são adaptações didáticas construídas para o escopo deste projeto.

| Faixa | Incidência |
|:------|:-----------|
| Baixo | < 50 |
| Médio | 50 – 99 |
| Alto | 100 – 299 |
| Crítico | ≥ 300 |

---

## Paradigmas aplicados

| Paradigma | Onde é aplicado |
|:----------|:----------------|
| **Orientado a objetos** | Classes com encapsulamento e invariantes internas |
| **Imperativo** | Menus, controle de fluxo, persistência em disco |
| **Funcional** | `std::accumulate`, `std::count_if`, `std::transform`, funções puras |

### Estruturas de controle

- `if / else` — validação de entrada, classificação de risco
- `switch / case` — rótulos de risco, menus de navegação
- `while` — leitura validada, carregamento de arquivo
- `do while` — menu principal, confirmação de exportação
- `for (range-for)` — iterações sobre coleções

---

## Estrutura do código

```
Projeto SARA.cpp

  enum class Risco

  namespace anônimo (parte 1)   — utilitários e helpers
    · leitura validada (lerInt, lerLinha)
    · parse seguro (stoiSeguro)
    · helpers (slugify, semanaAtual, classificarRisco)
    · rótulos de risco (riscoLabel, riscoIcone)

  class RegistroDoenca          — série temporal de uma patologia
  class Bairro                  — população + patologias + métricas
  class SARA                    — coleção + persistência + import

  namespace anônimo (parte 2)   — funções puras, exportadores, menus
    · funções puras (ehRiscoCritico, ehRiscoAlto)
    · exportadores (CSV, Mermaid, JSON, executivo)
    · menus interativos

  int main()                    — modo batch + loop principal
```

---

## Dados de exemplo

Populações conforme **Censo 2022 (IBGE)** para Maceió:

| Bairro | População |
|:-------|----------:|
| Benedito Bentes | 110.746 |
| Tabuleiro do Martins | 61.194 |
| Ponta Verde | 28.591 |
| Pajuçara | 5.500 |
| Centro | 3.200 |

Casos registrados nas semanas epidemiológicas **2026-W35** a **2026-W39** para Dengue, Zika, Chikungunya e Covid-19.

### Referências

- **IBGE** — Censo 2022: dados por bairro de Maceió
- **OMS** — limiar epidemiológico: ≥ 300 casos / 100 mil hab.
- **Ministério da Saúde** — Informe DAS Alagoas 2026

---

## Autor

Caick José Correia dos Santos
Projeto acadêmico — 2026

## Licença

Este projeto está sob a licença **MIT**. Veja o arquivo [LICENSE](LICENSE) para detalhes.
