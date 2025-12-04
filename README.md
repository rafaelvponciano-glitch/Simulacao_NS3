# Simulacao_NS3

Avaliação de Desempenho de Redes IoT em Ambiente Urbano (NS-3)

Equipe: Rafael Ponciano Vasceoncelos da Silva; Marco Antônio Oliveira Machado; Igor Rafael Carvalho Gonçalves.

🎯 Objetivo do Projeto

Este projeto de engenharia de telecomunicações visa avaliar e comparar o desempenho de uma rede de sensores de Internet das Coisas (IoT) em um ambiente urbano simulado. O foco é analisar como a variação de parâmetros físicos (Potência de Transmissão) e parâmetros de tráfego (Intervalo de Pacotes) impacta a confiabilidade (PDR) e a latência (Atraso) da rede.

A simulação foi implementada no Network Simulator 3 (NS-3) e é detalhada no script iot.cc.

🚀 Como Executar a Simulação

Este guia assume que o NS-3 está instalado e configurado corretamente.

Pré-requisitos
NS-3 versão 3.39 ou superior.

O script iot.cc deve estar na pasta scratch/ do diretório raiz do NS-3.

Passo a Passo:

Navegar para o Diretório Raiz do NS-3:

cd /caminho/para/seu/diretorio-ns3

Compilar o Projeto:

./ns3 build

Executar o Experimento:

O script rodará todos os 12 cenários automaticamente.
./ns3 --run "scratch/iot"

📊 Resultados e Conclusão Principal
A execução do script iot.cc gera o arquivo metrics.csv.
