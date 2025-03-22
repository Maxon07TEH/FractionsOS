#!/bin/bash

# Удаление старых директорий
rm -rf isodir efi createproject.sh OSInfo.txt

# Создание новой структуры
mkdir -p \
    .github \
    build/{bin,iso,obj} \
    src/{boot,drivers,kernel/x64} \
    scripts

# Перенос существующих файлов в новую структуру (без перезаписи)
[ ! -f src/boot/grub.cfg ] && cp -n boot/grub/grub.cfg src/boot/
[ ! -f src/drivers/vga.h ] && cp -n drivers/vga.h src/drivers/
[ ! -f src/drivers/vga.c ] && cp -n drivers/vga.c src/drivers/
[ ! -f src/kernel/x64/boot.asm ] && cp -n kernel/x64/boot.asm src/kernel/x64/
[ ! -f src/kernel/x64/main.c ] && cp -n kernel/x64/main.c src/kernel/x64/

# Создание скрипта публикации
cat > scripts/deploy-to-github.sh << 'EOF'
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
EOF

# Делаем скрипт исполняемым
chmod +x scripts/deploy-to-github.sh

# Уведомление о завершении
echo "Project structure updated!"
echo "1. Review moved files in src/"
echo "2. Edit scripts/deploy-to-github.sh (replace YOUR-USERNAME)"
echo "3. Run: ./scripts/deploy-to-github.sh"
