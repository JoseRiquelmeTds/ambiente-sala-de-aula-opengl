# Relatório da AB2

Relatório técnico do projeto em LaTeX, no padrão do **Manual para normalização de trabalhos acadêmicos da UFAL**
(SIBI/UFAL, 2. ed., 2025), sobre a classe abnTeX2.

| Arquivo | Conteúdo |
| --- | --- |
| [relatorio.pdf](relatorio.pdf) | Relatório exportado (versão de entrega) |
| `relatorio.tex` | Fonte do relatório |
| `referencias.bib` | Referências (biblatex-abnt: NBR 6023 e citação autor-data da NBR 10520:2023) |
| `figuras/` | Capturas do programa, feitas a partir de um clone limpo da v1.0.0 |

## Formatação aplicada

Seguindo o manual da UFAL (seção 3.2.1 e Figuras 2, 7 e 8):

- A4, margens de 3 cm (superior e esquerda) e 2 cm (inferior e direita), só anverso;
- Times (TeX Gyre Termes) 12 no texto, entrelinha 1,5 e espaço simples em legendas, códigos, quadros e referências;
- numeração no canto superior direito a partir da introdução, contando desde a folha de rosto;
- seção primária em negrito e caixa alta, secundária em negrito, terciária sem negrito;
- legenda acima de figuras, quadros e códigos, com "Fonte: ..." abaixo;
- capa (instituição, autores, título, local e ano) e folha de rosto (autores, título, natureza, professor, local
  e ano). Ficha catalográfica e folha de aprovação ficam de fora: o manual as exige só para TCC, dissertação e
  tese depositados no RIUFAL.

## Compilar

Precisa de uma distribuição TeX com `abntex2`, `biblatex-abnt`, `biber` e `latexmk` (TeX Live completo ou
MiKTeX). Nesta pasta:

```bash
latexmk -pdf relatorio.tex
```

Sem TeX instalado, com Docker (troque `$PWD` por `${PWD}` no PowerShell):

```bash
docker run --rm -v "$PWD:/w" -w /w texlive/texlive:latest latexmk -pdf relatorio.tex
```

Também compila no Overleaf (compilador pdfLaTeX, com biber). Os arquivos intermediários (`.aux`, `.bbl`,
`.log` etc.) são ignorados pelo git; só o `relatorio.pdf` é versionado.
