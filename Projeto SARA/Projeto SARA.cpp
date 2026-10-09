#define _CRT_SECURE_NO_WARNINGS
// S.A.R.A. - Sistema de Análise e Rastreamento de Anomalias
// Vigilância Epidemiológica Inteligente
// Versão 4.0
//
// REFERÊNCIAS DE DOMÍNIO
//   IBGE Censo 2022.......  Maceió: dados por bairro
//   OMS...................  Limiar epidêmico: >= 300 casos / 100 mil hab.
//   Ministério da Saúde...  Informe DAS Alagoas 2026
//
// NOTA SOBRE AS FAIXAS DE RISCO
//   A OMS define apenas o limiar epidêmico (>= 300 / 100k).
//   As faixas intermediárias (Baixo, Médio, Alto) são adaptações
//   didáticas construídas para o escopo deste projeto.

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <cmath>
#include <cctype>
#include <ctime>
#include <limits>
#include <numeric>
#include <stdexcept>

using namespace std;

// ENUM: Categoria de risco

enum class Risco { BAIXO, MEDIO, ALTO, CRITICO };

// NAMESPACE ANÔNIMO (parte 1) — utilitários e helpers

namespace {

    void limparBuffer() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    int lerInt(const string& prompt, int minV = -999999999, int maxV = 999999999) {
        int v;
        while (true) {
            cout << prompt;
            if (cin >> v) {
                if (v >= minV && v <= maxV) {
                    limparBuffer();
                    return v;
                }
                cout << "  [!] Valor fora do intervalo [" << minV << ", " << maxV << "].\n";
            }
            else {
                cout << "  [!] Entrada invalida. Digite um numero inteiro.\n";
                limparBuffer();
            }
        }
    }

    string lerLinha(const string& prompt) {
        string s;
        while (true) {
            cout << prompt;
            getline(cin, s);
            size_t a = s.find_first_not_of(" \t");
            size_t b = s.find_last_not_of(" \t");
            if (a == string::npos) {
                cout << "  [!] Nao pode ser vazio.\n";
                continue;
            }
            return s.substr(a, b - a + 1);
        }
    }

    bool stoiSeguro(const string& s, int& out) {
        try {
            size_t pos;
            out = stoi(s, &pos);
            return pos == s.size();
        }
        catch (...) {
            return false;
        }
    }

    string slugify(const string& s) {
        string out;
        for (char c : s) if (isalnum((unsigned char)c)) out += c;
        return out.empty() ? "X" : out;
    }

    string semanaAtual() {
        time_t t = time(nullptr);
        tm lt_storage;
#ifdef _WIN32
        localtime_s(&lt_storage, &t);
#else
        localtime_r(&t, &lt_storage);
#endif
        int ano = lt_storage.tm_year + 1900;
        int semana = lt_storage.tm_yday / 7 + 1;
        ostringstream oss;
        oss << ano << "-W" << setw(2) << setfill('0') << semana;
        return oss.str();
    }

    string riscoLabel(Risco r) {
        switch (r) {
        case Risco::BAIXO:   return "BAIXO";
        case Risco::MEDIO:   return "MEDIO";
        case Risco::ALTO:    return "ALTO";
        case Risco::CRITICO: return "CRITICO";
        }
        return "?";
    }

    string riscoIcone(Risco r) {
        switch (r) {
        case Risco::BAIXO:   return "[ ]";
        case Risco::MEDIO:   return "[~]";
        case Risco::ALTO:    return "[!]";
        case Risco::CRITICO: return "[X]";
        }
        return "?";
    }

    // Faixas adaptadas da OMS.
    Risco classificarRisco(float inc) {
        if (inc < 50.0f)  return Risco::BAIXO;
        if (inc < 100.0f) return Risco::MEDIO;
        if (inc < 300.0f) return Risco::ALTO;
        return Risco::CRITICO;
    }

    string setaDirecional(float taxa) {
        if (taxa > 5.0f)  return "^";
        if (taxa < -5.0f) return "v";
        return "=";
    }

    vector<string> split(const string& s, char delim) {
        vector<string> out;
        stringstream ss(s);
        string item;
        while (getline(ss, item, delim)) out.push_back(item);
        return out;
    }

} // namespace anônimo (parte 1)

// CLASSES DE DOMÍNIO

class RegistroDoenca {
public:
    string nome;
    map<string, int> casosPorSemana;

    RegistroDoenca(const string& n) : nome(n) {}

    void registrar(const string& semana, int casos) {
        casosPorSemana[semana] = casos;
    }

    int casosAtuais() const {
        if (casosPorSemana.empty()) return 0;
        return casosPorSemana.rbegin()->second;
    }

    int casosSemanaAnterior() const {
        if (casosPorSemana.size() < 2) return 0;
        auto it = casosPorSemana.rbegin();
        ++it;
        return it->second;
    }

    float mediaMovel(int janela = 4) const {
        if (casosPorSemana.empty()) return 0.0f;
        vector<int> ultimas;
        int n = 0;
        for (auto it = casosPorSemana.rbegin();
            it != casosPorSemana.rend() && n < janela; ++it, ++n) {
            ultimas.push_back(it->second);
        }
        int soma = accumulate(ultimas.begin(), ultimas.end(), 0);
        return ultimas.empty() ? 0.0f : (float)soma / ultimas.size();
    }

    float taxaCrescimento() const {
        int ant = casosSemanaAnterior();
        if (ant == 0) return casosAtuais() > 0 ? 100.0f : 0.0f;
        return ((float)(casosAtuais() - ant) / ant) * 100.0f;
    }

    int totalHistorico() const {
        return accumulate(casosPorSemana.begin(), casosPorSemana.end(), 0,
            [](int acc, const pair<const string, int>& kv) {
                return acc + kv.second;
            });
    }

    vector<pair<string, int>> ultimasSemanas(int n = 8) const {
        vector<pair<string, int>> out(casosPorSemana.begin(), casosPorSemana.end());
        if ((int)out.size() > n) out.erase(out.begin(), out.end() - n);
        return out;
    }
};

class Bairro {
private:
    string nome;
    int populacao;
    vector<RegistroDoenca> patologias;
    float incidenciaTotal;
    Risco risco;

public:
    Bairro(const string& n, int pop)
        : nome(n), populacao(pop), incidenciaTotal(0.0f), risco(Risco::BAIXO) {
    }

    const string& getNome() const { return nome; }
    int getPopulacao() const { return populacao; }
    float getIncidencia() const { return incidenciaTotal; }
    Risco getRisco() const { return risco; }
    const vector<RegistroDoenca>& getPatologias() const { return patologias; }

    const RegistroDoenca* getPatologia(const string& nome) const {
        for (const auto& p : patologias)
            if (p.nome == nome) return &p;
        return nullptr;
    }

    void atualizarPatologia(const string& doenca, const string& semana, int casos) {
        for (auto& p : patologias) {
            if (p.nome == doenca) { p.registrar(semana, casos); return; }
        }
        RegistroDoenca novo(doenca);
        novo.registrar(semana, casos);
        patologias.push_back(novo);
    }

    int totalCasosAtuais() const {
        return accumulate(patologias.begin(), patologias.end(), 0,
            [](int acc, const RegistroDoenca& p) {
                return acc + p.casosAtuais();
            });
    }

    int totalHistorico() const {
        return accumulate(patologias.begin(), patologias.end(), 0,
            [](int acc, const RegistroDoenca& p) {
                return acc + p.totalHistorico();
            });
    }

    void recalcular() {
        int total = totalCasosAtuais();
        incidenciaTotal = (populacao > 0)
            ? ((float)total / populacao) * 100000.0f : 0.0f;
        risco = classificarRisco(incidenciaTotal);
    }

    float tendencia() const {
        if (patologias.empty()) return 0.0f;
        float soma = accumulate(patologias.begin(), patologias.end(), 0.0f,
            [](float acc, const RegistroDoenca& p) {
                return acc + p.taxaCrescimento();
            });
        return soma / patologias.size();
    }

    void exibirRelatorio() const {
        cout << "\n  " << riscoIcone(risco) << " " << left << setw(22) << nome
            << " | Pop: " << setw(9) << populacao
            << " | Casos: " << setw(5) << totalCasosAtuais()
            << " | Inc/100k: " << setw(8) << fixed << setprecision(1) << incidenciaTotal
            << " | " << riscoLabel(risco);

        float t = tendencia();
        cout << " | " << setaDirecional(t) << " ";
        if (t > 5.0f)       cout << showpos << setprecision(1) << t << noshowpos << "%";
        else if (t < -5.0f) cout << showpos << setprecision(1) << t << noshowpos << "%";
        else                cout << "estavel";
        cout << "\n";

        for (const auto& p : patologias) {
            int at = p.casosAtuais();
            int ant = p.casosSemanaAnterior();
            int delta = at - ant;

            cout << "      - " << left << setw(14) << p.nome
                << " casos: " << setw(4) << at
                << " (" << setaDirecional(p.taxaCrescimento()) << " "
                << showpos << delta << noshowpos << ")"
                << " | media 4sem: " << setw(6) << fixed << setprecision(1)
                << p.mediaMovel()
                << " | cresc: " << showpos << setprecision(1)
                << p.taxaCrescimento() << noshowpos << "%\n";
        }
    }

    void exibirGrafico(const string& nomePatologia, int larguraMax = 40) const {
        const RegistroDoenca* p = getPatologia(nomePatologia);
        if (!p) {
            cout << "  [!] Patologia '" << nomePatologia << "' nao encontrada em " << nome << ".\n";
            return;
        }
        auto serie = p->ultimasSemanas(8);
        if (serie.empty()) {
            cout << "  (sem dados historicos)\n";
            return;
        }

        int maxVal = max_element(serie.begin(), serie.end(),
            [](const pair<string, int>& a, const pair<string, int>& b) {
                return a.second < b.second;
            })->second;
        if (maxVal == 0) maxVal = 1;

        cout << "\n  " << nome << " | " << nomePatologia
            << " (ultimas " << serie.size() << " semanas)\n";
        cout << "  " << string(larguraMax + 20, '-') << "\n";

        for (size_t i = 0; i < serie.size(); ++i) {
            const auto& [sem, val] = serie[i];
            int barras = (int)round((double)val / maxVal * larguraMax);
            cout << "  " << left << setw(8) << sem << " "
                << string(barras, '#')
                << " " << val;
            if (i == serie.size() - 1) cout << "  <- atual";
            cout << "\n";
        }
        cout << "  " << string(larguraMax + 20, '-') << "\n";
    }

    vector<string> nomesPatologias() const {
        vector<string> out(patologias.size());
        transform(patologias.begin(), patologias.end(), out.begin(),
            [](const RegistroDoenca& p) { return p.nome; });
        return out;
    }
};

class SARA {
private:
    vector<Bairro> bairros;
    const string arquivoDB = "sara.db";
    const string arquivoBak = "sara.db.bak";

    Bairro* buscar(const string& nome) {
        for (auto& b : bairros) if (b.getNome() == nome) return &b;
        return nullptr;
    }

public:
    const vector<Bairro>& getBairros() const { return bairros; }
    int count() const { return (int)bairros.size(); }

    const Bairro* getBairro(const string& nome) const {
        for (const auto& b : bairros) if (b.getNome() == nome) return &b;
        return nullptr;
    }

    bool cadastrarBairro(const string& nome, int pop) {
        if (buscar(nome)) return false;
        bairros.emplace_back(nome, pop);
        bairros.back().recalcular();
        ordenar();
        return true;
    }

    bool removerBairro(const string& nome) {
        for (auto it = bairros.begin(); it != bairros.end(); ++it) {
            if (it->getNome() == nome) { bairros.erase(it); return true; }
        }
        return false;
    }

    bool registrarPatologia(const string& bairro, const string& doenca,
        const string& semana, int casos) {
        Bairro* b = buscar(bairro);
        if (!b) return false;
        b->atualizarPatologia(doenca, semana, casos);
        b->recalcular();
        ordenar();
        return true;
    }

    void ordenar() {
        sort(bairros.begin(), bairros.end(),
            [](const Bairro& a, const Bairro& b) {
                return a.getIncidencia() > b.getIncidencia();
            });
    }

    int totalBairros() const { return (int)bairros.size(); }

    int totalPopulacao() const {
        return accumulate(bairros.begin(), bairros.end(), 0,
            [](int acc, const Bairro& b) { return acc + b.getPopulacao(); });
    }

    int totalCasosAtivos() const {
        return accumulate(bairros.begin(), bairros.end(), 0,
            [](int acc, const Bairro& b) { return acc + b.totalCasosAtuais(); });
    }

    int countRisco(Risco r) const {
        return (int)count_if(bairros.begin(), bairros.end(),
            [r](const Bairro& b) { return b.getRisco() == r; });
    }

    void fazerBackup() {
        ifstream src(arquivoDB, ios::binary);
        if (!src.is_open()) return;
        ofstream dst(arquivoBak, ios::binary);
        dst << src.rdbuf();
    }

    void salvar() {
        fazerBackup();
        ofstream f(arquivoDB);
        if (!f.is_open()) {
            cout << "  [!] Falha ao salvar em " << arquivoDB << "\n";
            return;
        }
        f << "# SARA DB v4.0\n";
        f << "# BAIRRO|nome|populacao\n";
        f << "# PATOLOGIA|nomeBairro|nomeDoenca|semana|casos\n";

        for (const auto& b : bairros) {
            f << "BAIRRO|" << b.getNome() << "|" << b.getPopulacao() << "\n";
            for (const auto& p : b.getPatologias())
                for (const auto& par : p.casosPorSemana)
                    f << "PATOLOGIA|" << b.getNome() << "|" << p.nome
                    << "|" << par.first << "|" << par.second << "\n";
        }
        f.close();
        cout << "  [OK] Estado salvo em " << arquivoDB << " (backup: " << arquivoBak << ")\n";
    }

    void carregar() {
        ifstream f(arquivoDB);
        if (!f.is_open()) {
            cout << "  [i] Nenhum arquivo anterior. Iniciando base vazia.\n";
            return;
        }
        string linha;
        int linhasInvalidas = 0;
        while (getline(f, linha)) {
            if (linha.empty() || linha[0] == '#') continue;
            stringstream ss(linha);
            string tipo;
            getline(ss, tipo, '|');

            if (tipo == "BAIRRO") {
                string nome, popStr;
                getline(ss, nome, '|');
                getline(ss, popStr, '|');
                int pop;
                if (!stoiSeguro(popStr, pop)) {
                    cerr << "  [!] Linha invalida ignorada: " << linha << "\n";
                    linhasInvalidas++;
                    continue;
                }
                cadastrarBairro(nome, pop);
            }
            else if (tipo == "PATOLOGIA") {
                string nb, d, s, cs;
                getline(ss, nb, '|');
                getline(ss, d, '|');
                getline(ss, s, '|');
                getline(ss, cs, '|');
                int casos;
                if (!stoiSeguro(cs, casos)) {
                    cerr << "  [!] Linha invalida ignorada: " << linha << "\n";
                    linhasInvalidas++;
                    continue;
                }
                registrarPatologia(nb, d, s, casos);
            }
        }
        f.close();
        cout << "  [OK] Estado carregado de " << arquivoDB
            << " (" << bairros.size() << " bairros";
        if (linhasInvalidas > 0)
            cout << ", " << linhasInvalidas << " linhas invalidas ignoradas";
        cout << ")\n";
    }

    int importarCSV(const string& caminho) {
        ifstream f(caminho);
        if (!f.is_open()) {
            cout << "  [!] Nao foi possivel abrir " << caminho << "\n";
            return 0;
        }
        string linha;
        int importados = 0;
        int erros = 0;
        while (getline(f, linha)) {
            if (linha.empty() || linha[0] == '#') continue;
            auto campos = split(linha, ',');

            if (campos.size() == 2) {
                int pop;
                if (!stoiSeguro(campos[1], pop)) { erros++; continue; }
                cadastrarBairro(campos[0], pop);
                importados++;
            }
            else if (campos.size() == 4) {
                int casos;
                if (!stoiSeguro(campos[3], casos)) { erros++; continue; }
                registrarPatologia(campos[0], campos[1], campos[2], casos);
                importados++;
            }
            else {
                erros++;
            }
        }
        f.close();
        cout << "  [OK] " << importados << " linhas importadas de " << caminho;
        if (erros > 0) cout << " (" << erros << " ignoradas por formato invalido)";
        cout << "\n";
        return importados;
    }
};

// NAMESPACE ANÔNIMO (parte 2) — funções puras, exportadores e menus

namespace {

    bool ehRiscoCritico(const Bairro& b) {
        return b.getRisco() == Risco::CRITICO;
    }

    bool ehRiscoAlto(const Bairro& b) {
        return b.getRisco() == Risco::ALTO;
    }

    bool temCasosAtivos(const RegistroDoenca& p) {
        return p.casosAtuais() > 0;
    }

    int contarCriticos(const vector<Bairro>& bairros) {
        return (int)count_if(bairros.begin(), bairros.end(), ehRiscoCritico);
    }

    void exportarCSV(const vector<Bairro>& bairros) {
        ofstream f("SARA_Relatorio.csv");
        if (!f.is_open()) { cout << "  [!] Falha ao gerar CSV.\n"; return; }

        f << "Bairro,Populacao,Casos_Atuais,Casos_Historicos,"
            << "Incidencia_100k,Risco,Tendencia_%,Patologia,Casos_Patologia,"
            << "Media_Movel_4sem\n";

        for (const auto& b : bairros) {
            if (b.getPatologias().empty()) {
                f << b.getNome() << "," << b.getPopulacao() << ","
                    << b.totalCasosAtuais() << "," << b.totalHistorico() << ","
                    << b.getIncidencia() << "," << riscoLabel(b.getRisco())
                    << "," << b.tendencia() << ",,,,,\n";
            }
            else {
                for (const auto& p : b.getPatologias()) {
                    f << b.getNome() << "," << b.getPopulacao() << ","
                        << b.totalCasosAtuais() << "," << b.totalHistorico() << ","
                        << b.getIncidencia() << "," << riscoLabel(b.getRisco())
                        << "," << b.tendencia() << ","
                        << p.nome << "," << p.casosAtuais() << ","
                        << p.mediaMovel() << "\n";
                }
            }
        }
        f.close();
        cout << "  [OK] CSV gerado: SARA_Relatorio.csv\n";
    }

    void exportarMermaid(const vector<Bairro>& bairros) {
        ofstream f("SARA_Grafo.md");
        if (!f.is_open()) { cout << "  [!] Falha ao gerar grafo.\n"; return; }

        f << "# SARA - Grafo Epidemiologico\n\n";
        f << "Sistema de Analise e Rastreamento de Anomalias\n\n";
        f << "Gerado em: " << semanaAtual() << "\n\n";
        f << "```mermaid\ngraph TD\n";
        f << "  SUS((Secretaria de Saude))\n";

        for (const auto& b : bairros) {
            string id = slugify(b.getNome());
            string cor;
            switch (b.getRisco()) {
            case Risco::BAIXO:   cor = "#4caf50"; break;
            case Risco::MEDIO:   cor = "#ffc107"; break;
            case Risco::ALTO:    cor = "#ff9800"; break;
            case Risco::CRITICO: cor = "#f44336"; break;
            }
            f << "  SUS ==> " << id << "{" << b.getNome() << "}\n";
            f << "  style " << id << " fill:" << cor
                << ",stroke:#222,stroke-width:2px,color:#fff\n";

            for (const auto& p : b.getPatologias()) {
                if (p.casosAtuais() > 0) {
                    string idD = id + slugify(p.nome);
                    f << "  " << id << " -->|" << p.casosAtuais() << " casos| "
                        << idD << "(" << p.nome << ")\n";
                    if (p.casosAtuais() >= 100)
                        f << "  style " << idD
                        << " fill:#7b1fa2,stroke:#333,stroke-width:2px,color:#fff\n";
                    else if (p.casosAtuais() >= 50)
                        f << "  style " << idD
                        << " fill:#c2185b,stroke:#333,stroke-width:2px,color:#fff\n";
                }
            }
        }
        f << "```\n";
        f.close();
        cout << "  [OK] Grafo Mermaid: SARA_Grafo.md\n";
    }

    void exportarJSON(const vector<Bairro>& bairros) {
        ofstream f("SARA_Dados.json");
        if (!f.is_open()) { cout << "  [!] Falha ao gerar JSON.\n"; return; }

        f << "{\n  \"sistema\": \"S.A.R.A.\",\n";
        f << "  \"versao\": \"4.0\",\n";
        f << "  \"gerado_em\": \"" << semanaAtual() << "\",\n";
        f << "  \"bairros\": [\n";
        for (size_t i = 0; i < bairros.size(); ++i) {
            const auto& b = bairros[i];
            f << "    {\n";
            f << "      \"nome\": \"" << b.getNome() << "\",\n";
            f << "      \"populacao\": " << b.getPopulacao() << ",\n";
            f << "      \"incidencia_100k\": " << fixed << setprecision(2)
                << b.getIncidencia() << ",\n";
            f << "      \"risco\": \"" << riscoLabel(b.getRisco()) << "\",\n";
            f << "      \"tendencia_pct\": " << b.tendencia() << ",\n";
            f << "      \"patologias\": [\n";
            const auto& pats = b.getPatologias();
            for (size_t j = 0; j < pats.size(); ++j) {
                const auto& p = pats[j];
                f << "        {\"nome\": \"" << p.nome
                    << "\", \"casos_atuais\": " << p.casosAtuais()
                    << ", \"media_4sem\": " << p.mediaMovel() << "}";
                if (j + 1 < pats.size()) f << ",";
                f << "\n";
            }
            f << "      ]\n    }";
            if (i + 1 < bairros.size()) f << ",";
            f << "\n";
        }
        f << "  ]\n}\n";
        f.close();
        cout << "  [OK] JSON gerado: SARA_Dados.json\n";
    }

    void exportarRelatorioExecutivo(const vector<Bairro>& bairros, const SARA& sys) {
        ofstream f("SARA_Relatorio_Executivo.txt");
        if (!f.is_open()) { cout << "  [!] Falha ao gerar relatorio.\n"; return; }

        f << "==========================================================\n";
        f << "  S.A.R.A. - SISTEMA DE ANALISE E RASTREAMENTO DE ANOMALIAS\n";
        f << "  RELATORIO EXECUTIVO\n";
        f << "==========================================================\n";
        f << "Data de referencia: " << semanaAtual() << "\n\n";

        f << "--- SUMARIO ---\n";
        f << "Bairros monitorados: " << sys.totalBairros() << "\n";
        f << "Populacao total: " << sys.totalPopulacao() << "\n";
        f << "Casos ativos totais: " << sys.totalCasosAtivos() << "\n\n";

        f << "--- DISTRIBUICAO DE RISCO (OMS) ---\n";
        f << "Critico: " << sys.countRisco(Risco::CRITICO) << " bairros\n";
        f << "Alto:    " << sys.countRisco(Risco::ALTO) << " bairros\n";
        f << "Medio:   " << sys.countRisco(Risco::MEDIO) << " bairros\n";
        f << "Baixo:   " << sys.countRisco(Risco::BAIXO) << " bairros\n\n";

        f << "--- ACOES PRIORITARIAS ---\n";
        bool algum = false;

        int criticos = (int)count_if(bairros.begin(), bairros.end(), ehRiscoCritico);
        if (criticos > 0) {
            for (const auto& b : bairros) {
                if (ehRiscoCritico(b)) {
                    f << "[URGENTE] " << b.getNome()
                        << " - incidencia " << fixed << setprecision(1)
                        << b.getIncidencia()
                        << " (acionar equipe em 24h)\n";
                    algum = true;
                }
            }
        }

        int altos = (int)count_if(bairros.begin(), bairros.end(), ehRiscoAlto);
        if (altos > 0) {
            for (const auto& b : bairros) {
                if (ehRiscoAlto(b)) {
                    f << "[ALERTA]  " << b.getNome()
                        << " - incidencia " << fixed << setprecision(1)
                        << b.getIncidencia()
                        << " (monitoramento intensificado)\n";
                    algum = true;
                }
            }
        }
        if (!algum)
            f << "Nenhuma acao critica necessaria nesta semana.\n";

        f << "\n==========================================================\n";
        f << "Relatorio gerado automaticamente pelo S.A.R.A. 4.0\n";
        f << "==========================================================\n";
        f.close();
        cout << "  [OK] Relatorio executivo: SARA_Relatorio_Executivo.txt\n";
    }

    void exibirCabecalho(const SARA& sys) {
        cout << "\n+=================================================+\n";
        cout << "|   S.A.R.A. 4.0  -  Vigilancia Epidemiologica    |\n";
        cout << "|   Sistema de Analise e Rastreamento de Anomalias|\n";
        cout << "+-------------------------------------------------+\n";
        cout << "| Bairros: " << setw(3) << sys.totalBairros()
            << " | Pop: " << setw(10) << sys.totalPopulacao()
            << " | Casos: " << setw(5) << sys.totalCasosAtivos() << "   |\n";
        cout << "| Semana: " << setw(8) << semanaAtual()
            << " | Risco: " << sys.countRisco(Risco::CRITICO) << "C "
            << sys.countRisco(Risco::ALTO) << "A "
            << sys.countRisco(Risco::MEDIO) << "M "
            << sys.countRisco(Risco::BAIXO) << "B"
            << "        |\n";
        cout << "+=================================================+\n";
    }

    void menuCadastrar(SARA& sys) {
        cout << "\n--- CADASTRAR BAIRRO ---\n";
        string nome = lerLinha("Nome do bairro: ");
        int pop = lerInt("Populacao (habitantes): ", 1, 100000000);
        if (sys.cadastrarBairro(nome, pop))
            cout << "  [OK] Bairro cadastrado.\n";
        else
            cout << "  [!] Bairro ja existe. Cadastro ignorado.\n";
    }

    void menuRemover(SARA& sys) {
        if (sys.count() == 0) { cout << "  [!] Nenhum bairro cadastrado.\n"; return; }
        cout << "\n--- REMOVER BAIRRO ---\n";
        for (const auto& b : sys.getBairros()) cout << "  - " << b.getNome() << "\n";
        string nome = lerLinha("Nome do bairro a remover: ");
        if (sys.removerBairro(nome)) cout << "  [OK] Removido.\n";
        else cout << "  [!] Nao encontrado.\n";
    }

    void menuRegistrar(SARA& sys) {
        if (sys.count() == 0) { cout << "  [!] Nenhum bairro cadastrado.\n"; return; }
        cout << "\n--- REGISTRAR CASOS (semana atual: " << semanaAtual() << ") ---\n";
        string semana = lerLinha("Semana [Enter = " + semanaAtual() + "]: ");
        if (semana.empty()) semana = semanaAtual();
        string doenca = lerLinha("Nome da patologia (ex: Dengue): ");

        for (const auto& b : sys.getBairros()) {
            cout << "\n  Bairro: " << b.getNome() << "\n";
            int c = lerInt("  Casos confirmados: ", 0, 1000000);
            sys.registrarPatologia(b.getNome(), doenca, semana, c);
        }
        cout << "\n  [OK] Dados registrados e ranking atualizado.\n";
    }

    void menuRelatorio(const SARA& sys) {
        cout << "\n=============== RANKING DE RISCO ===============\n";
        if (sys.count() == 0) { cout << "  (sem dados)\n"; return; }
        for (const auto& b : sys.getBairros()) b.exibirRelatorio();
        cout << "================================================\n";
    }

    void menuGrafico(const SARA& sys) {
        if (sys.count() == 0) { cout << "  [!] Nenhum bairro cadastrado.\n"; return; }
        cout << "\n--- GRAFICO DE SERIE TEMPORAL ---\n";
        for (const auto& b : sys.getBairros()) {
            auto pats = b.nomesPatologias();
            if (pats.empty()) continue;
            for (const auto& p : pats) {
                b.exibirGrafico(p);
            }
        }
    }

    void menuImportar(SARA& sys) {
        cout << "\n--- IMPORTAR CSV ---\n";
        cout << "  Formato esperado (uma linha por registro):\n";
        cout << "    bairro,populacao\n";
        cout << "    bairro,patologia,semana,casos\n";
        cout << "  Linhas com # sao ignoradas.\n\n";
        string caminho = lerLinha("Caminho do arquivo CSV: ");
        sys.importarCSV(caminho);
    }

    void menuExportar(const SARA& sys) {
        bool continuar;
        do {
            cout << "\n--- EXPORTAR RELATORIOS ---\n";
            cout << "  1. CSV tabular\n";
            cout << "  2. Grafo Mermaid (Obsidian)\n";
            cout << "  3. JSON (integracao)\n";
            cout << "  4. Relatorio executivo (.txt)\n";
            cout << "  5. Todos\n";
            cout << "  0. Cancelar\n";
            int op = lerInt("Opcao: ", 0, 5);
            switch (op) {
            case 1: exportarCSV(sys.getBairros()); break;
            case 2: exportarMermaid(sys.getBairros()); break;
            case 3: exportarJSON(sys.getBairros()); break;
            case 4: exportarRelatorioExecutivo(sys.getBairros(), sys); break;
            case 5:
                exportarCSV(sys.getBairros());
                exportarMermaid(sys.getBairros());
                exportarJSON(sys.getBairros());
                exportarRelatorioExecutivo(sys.getBairros(), sys);
                break;
            case 0:
                cout << "  Cancelado.\n";
                continuar = false;
                continue;
            }

            string resp = lerLinha("Deseja exportar mais alguma coisa? (s/n): ");
            continuar = (resp == "s" || resp == "S" || resp == "sim" || resp == "SIM");
        } while (continuar);
    }

    int modoBatch(SARA& sys) {
        cout << "[MODO BATCH]\n";
        sys.carregar();

        if (sys.count() == 0) {
            cerr << "  [!] Base vazia. Nada a exportar.\n";
            return 1;
        }

        exportarCSV(sys.getBairros());
        exportarMermaid(sys.getBairros());
        exportarJSON(sys.getBairros());
        exportarRelatorioExecutivo(sys.getBairros(), sys);
        cout << "[BATCH] Exportacao concluida.\n";
        return 0;
    }

} // namespace anônimo (parte 2)

// MAIN

int main(int argc, char* argv[]) {
    if (argc > 1) {
        string arg = argv[1];
        if (arg == "--batch" || arg == "--export") {
            SARA sys;
            return modoBatch(sys);
        }
        if (arg == "--help" || arg == "-h") {
            cout << "Uso:\n";
            cout << "  Projeto_SARA.exe               Modo interativo\n";
            cout << "  Projeto_SARA.exe --batch       Exporta tudo e sai\n";
            cout << "  Projeto_SARA.exe --help        Esta ajuda\n";
            return 0;
        }
    }

    SARA sys;
    sys.carregar();

    if (sys.count() == 0) {
        cout << "  [i] Populando base de exemplo (Censo 2022 - IBGE)...\n";

        // Populacao dos bairros (Censo 2022 - IBGE)
        sys.cadastrarBairro("Benedito Bentes", 110746);
        sys.cadastrarBairro("Tabuleiro do Martins", 61194);
        sys.cadastrarBairro("Ponta Verde", 28591);
        sys.cadastrarBairro("Pajucara", 5500);
        sys.cadastrarBairro("Centro", 3200);

        // SEMANAS EPIDEMIOLOGICAS 2026 (SE 35 a SE 39)
        // Dados de referencia: Informe DAS Alagoas 2026
        string s35 = "2026-W35";
        string s36 = "2026-W36";
        string s37 = "2026-W37";
        string s38 = "2026-W38";
        string s39 = "2026-W39";

        // DENGUE
        sys.registrarPatologia("Benedito Bentes", "Dengue", s35, 48);
        sys.registrarPatologia("Benedito Bentes", "Dengue", s36, 52);
        sys.registrarPatologia("Benedito Bentes", "Dengue", s37, 55);
        sys.registrarPatologia("Benedito Bentes", "Dengue", s38, 58);
        sys.registrarPatologia("Benedito Bentes", "Dengue", s39, 60);

        sys.registrarPatologia("Tabuleiro do Martins", "Dengue", s35, 38);
        sys.registrarPatologia("Tabuleiro do Martins", "Dengue", s36, 42);
        sys.registrarPatologia("Tabuleiro do Martins", "Dengue", s37, 45);
        sys.registrarPatologia("Tabuleiro do Martins", "Dengue", s38, 47);
        sys.registrarPatologia("Tabuleiro do Martins", "Dengue", s39, 50);

        sys.registrarPatologia("Ponta Verde", "Dengue", s35, 9);
        sys.registrarPatologia("Ponta Verde", "Dengue", s36, 10);
        sys.registrarPatologia("Ponta Verde", "Dengue", s37, 9);
        sys.registrarPatologia("Ponta Verde", "Dengue", s38, 11);
        sys.registrarPatologia("Ponta Verde", "Dengue", s39, 10);

        sys.registrarPatologia("Pajucara", "Dengue", s35, 6);
        sys.registrarPatologia("Pajucara", "Dengue", s36, 7);
        sys.registrarPatologia("Pajucara", "Dengue", s37, 8);
        sys.registrarPatologia("Pajucara", "Dengue", s38, 8);
        sys.registrarPatologia("Pajucara", "Dengue", s39, 8);

        sys.registrarPatologia("Centro", "Dengue", s35, 8);
        sys.registrarPatologia("Centro", "Dengue", s36, 9);
        sys.registrarPatologia("Centro", "Dengue", s37, 10);
        sys.registrarPatologia("Centro", "Dengue", s38, 10);
        sys.registrarPatologia("Centro", "Dengue", s39, 10);

        // ZIKA
        sys.registrarPatologia("Benedito Bentes", "Zika", s35, 2);
        sys.registrarPatologia("Benedito Bentes", "Zika", s36, 3);
        sys.registrarPatologia("Benedito Bentes", "Zika", s37, 2);
        sys.registrarPatologia("Benedito Bentes", "Zika", s38, 3);
        sys.registrarPatologia("Benedito Bentes", "Zika", s39, 3);

        sys.registrarPatologia("Centro", "Zika", s35, 1);
        sys.registrarPatologia("Centro", "Zika", s36, 1);
        sys.registrarPatologia("Centro", "Zika", s37, 2);
        sys.registrarPatologia("Centro", "Zika", s38, 2);
        sys.registrarPatologia("Centro", "Zika", s39, 2);

        // CHIKUNGUNYA
        sys.registrarPatologia("Tabuleiro do Martins", "Chikungunya", s35, 5);
        sys.registrarPatologia("Tabuleiro do Martins", "Chikungunya", s36, 6);
        sys.registrarPatologia("Tabuleiro do Martins", "Chikungunya", s37, 7);
        sys.registrarPatologia("Tabuleiro do Martins", "Chikungunya", s38, 8);
        sys.registrarPatologia("Tabuleiro do Martins", "Chikungunya", s39, 9);

        sys.registrarPatologia("Ponta Verde", "Chikungunya", s35, 1);
        sys.registrarPatologia("Ponta Verde", "Chikungunya", s36, 2);
        sys.registrarPatologia("Ponta Verde", "Chikungunya", s37, 1);
        sys.registrarPatologia("Ponta Verde", "Chikungunya", s38, 2);
        sys.registrarPatologia("Ponta Verde", "Chikungunya", s39, 2);

        // COVID-19
        sys.registrarPatologia("Centro", "Covid-19", s35, 4);
        sys.registrarPatologia("Centro", "Covid-19", s36, 5);
        sys.registrarPatologia("Centro", "Covid-19", s37, 4);
        sys.registrarPatologia("Centro", "Covid-19", s38, 5);
        sys.registrarPatologia("Centro", "Covid-19", s39, 6);

        sys.registrarPatologia("Ponta Verde", "Covid-19", s35, 2);
        sys.registrarPatologia("Ponta Verde", "Covid-19", s36, 3);
        sys.registrarPatologia("Ponta Verde", "Covid-19", s37, 2);
        sys.registrarPatologia("Ponta Verde", "Covid-19", s38, 3);
        sys.registrarPatologia("Ponta Verde", "Covid-19", s39, 3);
    }

    int opcao;
    do {
        exibirCabecalho(sys);
        cout << "  1. Cadastrar bairro\n";
        cout << "  2. Registrar casos (patologia + semana)\n";
        cout << "  3. Ver ranking de risco\n";
        cout << "  4. Exportar relatorios\n";
        cout << "  5. Remover bairro\n";
        cout << "  6. Salvar estado agora\n";
        cout << "  7. Grafico de serie temporal (ASCII)\n";
        cout << "  8. Importar CSV em batch\n";
        cout << "  0. Sair (salva automaticamente)\n";
        opcao = lerInt("  >> ");

        switch (opcao) {
        case 1: menuCadastrar(sys); break;
        case 2: menuRegistrar(sys); break;
        case 3: menuRelatorio(sys); break;
        case 4: menuExportar(sys); break;
        case 5: menuRemover(sys); break;
        case 6: sys.salvar(); break;
        case 7: menuGrafico(sys); break;
        case 8: menuImportar(sys); break;
        case 0:
            sys.salvar();
            cout << "\n  Encerrando. O SUS agradece.\n";
            break;
        default:
            cout << "  [!] Opcao invalida.\n";
        }
    } while (opcao != 0);

    return 0;
}