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

### Логика kettlebell — архитектурная схема

```text
┌─────────────────────────────────────────────────────────────────┐
│                         MAIN                                    │
├─────────────────────────────────────────────────────────────────┤
│  1. Парсинг аргументов командной строки                         │
│  2. Валидация комбинаций флагов                                 │
│  3. Диспетчеризация в соответствующий режим                     │
└─────────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        ▼                     ▼                     ▼
   ┌─────────┐          ┌──────────┐          ┌─────────┐
   │ --new   │          │ --post   │          │ --lang  │
   │ создать │          │ ген.     │          │ или     │
   │ шаблон  │          │ 1 пост   │          │ полная  │
   └─────────┘          └──────────┘          └─────────┘
        │                     │                     │
        ▼                     ▼                     ▼
   ┌─────────┐          ┌──────────┐          ┌─────────┐
   │ Генера- │          │ Проверка │          │ Обход   │
   │ ция md  │          │ сущест-  │          │ языков  │
   │ файлов  │          │ вования  │          │         │
   └─────────┘          └──────────┘          └─────────┘
                              │                     │
                              ▼                     ▼
                        ┌──────────┐          ┌─────────┐
                        │ Парсинг  │          │ Обход   │
                        │ фронт-   │          │ постов  │
                        │ материи  │          │ в языке │
                        └──────────┘          └─────────┘
                              │                     │
                              └──────────┬──────────┘
                                         ▼
                              ┌─────────────────────┐
                              │ Генерация HTML:     │
                              │ - подстановка мета  │
                              │ - подстановка config│
                              │ - кросс-ссылки      │
                              └─────────────────────┘
                                         │
                                         ▼
                              ┌──────────────────────┐
                              │ Обновление RSS:      │
                              │ - пересборка feed.xml│
                              │ - сортировка по дате │
                              └──────────────────────┘
                                         │
                                         ▼
                              ┌──────────────────────┐
                              │ Обновление index.html│
                              │ (список постов)      │
                              └──────────────────────┘
```

#### Режимы работы (диспетчеризация)

**Режим 1: ```--new <slug>```**
```text
--new last-bastion

Действие:
1. Проверить, что slug корректен (буквы, цифры, дефис)
2. Для каждого языка (ru, en, de):
   - Создать файл src/{lang}/posts/current-date-{slug}.md
   - Заполнить шаблоном с frontmatter
3. Вывести список созданных файлов
4. Exit(0)
```

Шаблон нового поста:
```markdown
---
title: TITLE_PLACEHOLDER
description: DESCRIPTION_PLACEHOLDER
lang: {lang}
slug: {slug}
related: { ru: {slug}, en: {slug}, de: {slug} }
---

Напишите текст здесь.
```

**Режим 2: ```--post {lang}/{slug}```**
```text
--post ru/new-idea

Действие:
1. Проверить формат (должен быть "/" внутри)
2. Извлечь lang и slug
3. Сформировать путь src/{lang}/posts/{slug}.md
4. Если файла нет → Error: source file not found
5. Если файл есть → вызвать generate_one_post(lang, slug, force_flag)
6. Если force_flag == 0 и build/{lang}/posts/{slug}.html существует → Error: already exists
7. Если force_flag == 1 → перезаписать
8. После генерации поста → обновить RSS для этого языка
9. После RSS → обновить index.html для этого языка
```

**Режим 3: ```--lang {lang}``` (с опциональным ```--force```)**
```text
--lang ru --force

Действие:
1. Валидировать lang (должен быть ru/en/de)
2. Если --clean указан → удалить build/{lang}/
3. Обойти директорию src/{lang}/posts/*.md
4. Для каждого поста:
   - Вызвать generate_one_post(lang, slug, force_flag)
5. После всех постов → сгенерировать RSS для языка
6. Сгенерировать index.html для языка
```

**Режим 4: Полная сборка (без флагов)**
```text
./kettlebell

Действие:
1. Для каждого языка (ru, en, de):
   - Обойти src/{lang}/posts/*.md
   - Для каждого поста:
        - если force_flag == 0 и файл существует в build → пропустить
        - иначе → сгенерировать
2. После всех постов → сгенерировать RSS для всех языков
3. Сгенерировать index.html для всех языков
4. Скопировать assets/ → build/assets/
```

#### Структура данных в памяти
```asm
post:
    .title         string
    .description   string
    .lang          string (ru/en/de)
    .slug          string
    .date          string (YYYY-MM-DD)
    .related_ru    string
    .related_en    string
    .related_de    string
    .content       string (HTML после парсинга)
    
config:
    .site_url      string
    .bot_name      string
    .threema_id    string
    .support_email string
```

#### Псевдокод основного цикла
```c
int main(int argc, char **argv) {
    parse_args(argc, argv);
    validate_flags();
    load_config();
    
    if (mode_new) {
        create_post_template(slug);
        exit(0);
    }
    
    if (mode_clean && !mode_force) {
        error("--clean requires --force");
        exit(1);
    }
    
    if (mode_post) {
        if (!file_exists(source_path)) error("source not found");
        if (!mode_force && file_exists(build_path)) error("already exists");
        generate_one_post(lang, slug, mode_force);
        update_rss_for_lang(lang);
        update_index_for_lang(lang);
        exit(0);
    }
    
    if (mode_lang) {
        if (mode_clean) remove_build_dir(lang);
        process_language(lang, mode_force);
        exit(0);
    }
    
    // full build
    if (mode_clean) remove_build_dir("all");
    process_all_languages(mode_force);
    copy_assets();
    exit(0);
}
```


#### Что происходит с RSS при --post
Просто пересобираем весь feed -- на ассемблере это доли секунд даже для 100 постов.

### Сисколы для гири

Системные вызовы Solaris для kettlebell
|  №  |    Сискол	     |     Название	    |                     Назначение                         |
|:----|:-----------------|:-----------------|:-------------------------------------------------------|
| 3	  | SYS_read         | sys_read	        | Чтение .md файлов, шаблонов                            |
| 4	  | SYS_write        | sys_write	    | Запись HTML, RSS, index.html                           |
| 68  | SYS_openat       | sys_open	        | Открытие файлов (O_RDONLY, O_WRONLY	O_CREAT, O_RDWR) |
| 6	  | SYS_close        | sys_close	    | Закрытие дескрипторов                                  |
| 35  | SYS_statatfs     | sys_stat	        | Проверка существования файла (--force check)           |
| 115 |	SYS_mmap	     | sys_mmap	        | Маппинг файлов в память (быстрый парсинг)              |
| 117 |	SYS_munmap	     | sys_munmap	    | Освобождение памяти                                    |
| 17  |	SYS_brk	         | sys_brk	        | Управление кучей (если не используем mmap)             |
| 81  |	SYS_getdents     | sys_getdents     | Чтение директории src/{lang}/posts/*.md                |
| 102 |	SYS_mkdirat	     | sys_mkdir	    | Создание build/ и поддиректорий                        |
| 1	  | SYS_exit	     | sys_exit	        | Выход с кодом ошибки                                   |
| 156 |	SYS_gettimeofday | sys_gettimeofday | Время для RSS lastBuildDate                            |

Дополнительные вызовы для работы с директориями
Для opendir/readdir используется SYS_getdents:

```asm
mov $78, %rax        # SYS_getdents
mov $fd, %rdi        # дескриптор директории
mov $buffer, %rsi    # буфер для dirent
mov $1024, %rdx      # размер буфера
syscall
```

### Рефы

|                               |                                                                                                                                  |  
|:------------------------------|:---------------------------------------------------------------------------------------------------------------------------------|
| Номера сисколов Solaris 11.4  | /usr/include/sys/syscall.h (на Solaris)                                                                                          |
