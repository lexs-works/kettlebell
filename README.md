![kettlebell · IN DEVELOPMENT](https://img.shields.io/badge/kettlebell-WIP-c8a86b?style=for-the-badge&logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIyMCIgaGVpZ2h0PSIyMCIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSJub25lIiBzdHJva2U9IiNjOGE4NmIiIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIj48Y2lyY2xlIGN4PSIxMiIgY3k9IjEyIiByPSIxMCIvPjxwYXRoIGQ9Ik0xMiA4djRsMiAyIi8+PC9zdmc+)
# Kettlebell - статический генератор блога, для лога коммерческой рефлексии https://log.lex.sale

## Инженерный черновик

### Флаги и примеры команд

| Флаг       |    Агументы   |                     Описание                        |
|:-----------|:--------------|----------------------------------------------------:|
| --force    |               | Перезаписать существующие посты
| --lang     | {lang}        | Сборка только для языка {lang}
| --clean    |               | Удалить ./build перед сборкой
| --new      | {slug}        | Создать файлы md для заметки, в существующих языках
| --post     | {lang}/{slug} | Собрать конкретный пост на конкретном языке

Полный набор команд для правильного использования
```sh
# Полная сборка всех новых статей
> ./kettlebell
# Пересобрать все статьи
> ./kettlebell --force
# Пересобрать все статьи, предварительно удалив папку ./build
> ./kettlebell --clean --force
# Собрать статьи только для 1 языка, только новые
> ./kettlebell --lang ru
# Пересобрать все статьи для 1 языка
> ./kettlebell --lang ru --force
# Генерация только 1 поста для 1 языка
> ./kettlebell --post ru/new-idea
# Перезаписать существующий пост или элемент в RSS
> ./kettlebell --post ru/llm-as-lvr --force
# Создать блан для поста во всех языках
> ./kettlebell --new last-bastion
```
Набор команд для контроллируемой ошибки:
```sh
# Пост для генерации отсутствует
> ./kettlebell --post ru/not-found
Error: source file not found: src/ru/posts/not-found.md
# Ошибка в формате
> ./kettlebell --post en/llm-as-lvr
Error: malformed source file: src/en/posts/llm-as-lvr

# Ошибка в формате команды post
> ./kettlebell --post enllm-as-lvr
Error: invalid --post format. Expected {lang}/{slug}

# Пост уже есть (запуск без --force)
> ./kettlebell --post de/llm-as-lvr
Error: post 'src/de/posts/llm-as-lvr' already exists

# Пересборка с нуля -- только принудительно
> ./kettlebell --clean
Error: --clean requires --force
```

### Логика работы

Файловая структура на входе:
```text
src/
├── ru/
│   ├── index.tmpl                  ← шаблон главной
│   ├── post.tmpl                   ← шаблон поста
│   ├── feed.xml.tmpl               ← шаблон RSS
│   └── posts/
│       ├── 2026-06-09-llm-as-lvr.md
│       └── 2026-06-08-pain-is-gypsy-talk.md
├── en/
│   ├── index.tmpl
│   ├── post.tmpl
│   ├── feed.xml.tmpl
│   └── posts/
│       ├── 2026-06-09-llm-as-dm.md
│       └── ...
└── de/
    ├── index.tmpl
    ├── post.tmpl
    ├── feed.xml.tmpl
    └── posts/
        └── ...
```

Файловая структура после сборки:
```text
build/
├── ru/
│   ├── index.html
│   ├── feed.xml
│   └── posts/
│       ├── llm-as-lvr.html
│       └── pain-is-gypsy-talk.html
├── en/
│   ├── index.html
│   ├── feed.xml
│   └── posts/
│       └── llm-as-dm.html
├── de/
│   ├── index.html
│   ├── feed.xml
│   └── posts/
│       └── llm-as-dm.html
└── assets/
    ├── log-preview-ru.png
    ├── log-preview-en.png
    └── log-preview-de.png
```


Общая архитектура:

  1. Чтение конфига (config.inc)
  2. Парсинг аргументов командной строки (--force, --lang, --post, --new)

     3.1 Если --post

     3.2 Если --new

     3.3 --foce, --lang или без аргумента
     	 
        3.3.1 Обход src/{lang}/posts/*.md                                           
  	    3.3.2 Для каждого поста:                                                    
     	      - Проверка: существует ли build/{lang}/posts/{slug}.html              
     	      - Если существует и нет флага --force → пропустить                    
     	      - Если флаг --force → перезаписать                                    
  	    3.3.3 Генерация HTML (с мета-тегами, OG, Twitter)                           
  	    
        3.3.4 Генерация RSS для каждого языка                                       
  	     3.3.5 Генерация index.html для каждого языка                                
  	    3.3.6 Копирование ассетов (assets/* → build/assets/)                        


Формат поста (frontmatter + тело):
```text
---
title: ЛПР умер. Да здравствует LLM-as-a-Decision-Maker
description: Руководитель советуется с нейросетью или делегирует ей решение. Как это меняет архитектуру B2B-продаж.
lang: ru
slug: llm-as-lvr
related: { en: llm-as-dm, de: llm-as-dm }
---

Текст поста...
```
Поля:

* title — заголовок (в ```<h1>``` и ```<title>```)
* description — мета-описание
* lang — язык (ru/en/de)
* slug — идентификатор (для URL и связей)
* related — slug той же статьи на других языках

Если related не указан — kettlebell пытается найти файл с тем же slug в другой языковой папке.




#### Что происходит с RSS при --post
Просто пересобираем весь feed -- на ассемблере это доли секунд даже для 100 постов.