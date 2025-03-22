#!/bin/bash

# Инициализация репозитория
if [ ! -d ".git" ]; then
    git init
    echo "# FractionsOS" > README.md
    git remote add origin git@github.com:YOUR-USERNAME/FractionsOS.git
fi

# Добавление файлов новой структуры
git add .
git commit -m "Project restructured: build system, src folder, scripts"

# Принудительный пуш (замените main на вашу ветку)
git branch -M main
git push -f -u origin main

echo "Project structure updated and pushed to GitHub!"
