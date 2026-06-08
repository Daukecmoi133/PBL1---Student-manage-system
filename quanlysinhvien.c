#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <windows.h>
#include "quanlysinhvien.h"

int DaSapXep = 0;
char CurrentFileName[1000] = "";

void save_last_filename() {
    FILE *f = fopen("last_file.txt", "w");
    if (f) {
        fprintf(f, "%s", CurrentFileName);
        fclose(f);
    }
}

void load_last_filename() {
    FILE *f = fopen("last_file.txt", "r");
    if (f) {
        if (fgets(CurrentFileName, sizeof(CurrentFileName), f)) {
            CurrentFileName[strcspn(CurrentFileName, "\r\n")] = '\0';
        }
        fclose(f);
    }
    if (strlen(CurrentFileName) == 0) {
        strcpy(CurrentFileName, "sinhvien.txt");
    }
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void clearScreen() {
    printf("\033[2J\033[0;0H");
}

void loading1s() {
    char spinner[] = "|/-\\";
    int colors[] = {10, 11, 14, 12, 13};

    int i = 0;
    DWORD start = GetTickCount();

    printf("Dang xu ly ");

    while (GetTickCount() - start < 1000) {
        setColor(colors[i % 5]);
        printf("%c", spinner[i % 4]);

        fflush(stdout);
        Sleep(100);

        printf("\b");
        i++;
    }

    setColor(10);
    printf("✔\n");

    setColor(7);
}

void vietHoaTatCa(char *s) {
    for (int i = 0; s[i] != '\0'; i++) {
        s[i] = toupper((unsigned char)s[i]);
    }
}

void vietHoaChuDau(char *s){
    for (int i = 0; s[i] != '\0'; i++){
        if (i == 0 || s[i-1] == ' ')
            s[i] = toupper((unsigned char) s[i]);
        else if ('A' <= s[i] <= 'Z') s[i] = tolower((unsigned char) s[i]);
    }
}
void xoaBoNhoDem()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int nhapSoNguyen()
{
    int n;
    while (scanf("%d", &n) != 1) {
        setColor(12);
        printf("Loi! Moi nhap lai: ");
        xoaBoNhoDem();
    }
    return n;
}
int checkClass(char* s){
    if (strlen(s) == 0) return 0;

    int value = atoi(s);
    if (value < 1 || value > 25) return 0;

    return 1;
}
void TachLop(char* s, char* sInt, char* sStr){

    int i = 0;
    int j = 0;
    int k = 0;
    int bol = 0;

    while (i < strlen(s)){

        if (bol == 0){

            if (!isdigit(s[i]))
                bol = 1;
            else
                sInt[j++] = s[i];
        }

        if (bol == 1)
            sStr[k++] = s[i];

        i++;
    }

    sInt[j] = '\0';
    sStr[k] = '\0';
}

static int class_prefix_matches(const char *str, const char *prefix) {
    size_t len = strlen(prefix);
    if (strncmp(str, prefix, len) != 0) return 0;
    return str[len] == '\0' || isdigit((unsigned char)str[len]) || str[len] == '_' || str[len] == '-';
}

int layMaKhoaTuLop(const char *className)
{
    const char *rest = className;
    while (*rest && isdigit((unsigned char)*rest)) rest++;
    if (*rest == '\0') return 0;

    if (class_prefix_matches(rest, "C1") || class_prefix_matches(rest, "CKHK") || class_prefix_matches(rest, "CDT")) return 101;
    if (class_prefix_matches(rest, "T_DT") || class_prefix_matches(rest, "KHDL") || class_prefix_matches(rest, "NHAT")) return 102;
    if (class_prefix_matches(rest, "C4") || class_prefix_matches(rest, "KTOTO") || class_prefix_matches(rest, "KTTT") || class_prefix_matches(rest, "HTCN")) return 103;
    if (class_prefix_matches(rest, "N") || class_prefix_matches(rest, "QLNL")) return 104;
    if (class_prefix_matches(rest, "D") || class_prefix_matches(rest, "TDH")) return 105;
    if (class_prefix_matches(rest, "DT") || class_prefix_matches(rest, "KTMT")) return 106;
    if (class_prefix_matches(rest, "H2") || class_prefix_matches(rest, "KTHH") || class_prefix_matches(rest, "H5") || class_prefix_matches(rest, "SH") || class_prefix_matches(rest, "SHYD")) return 107;
    if (class_prefix_matches(rest, "X3") || class_prefix_matches(rest, "BIM_AI") || class_prefix_matches(rest, "VLXD") || class_prefix_matches(rest, "X3_DS")) return 108;
    if (class_prefix_matches(rest, "X1")) return 109;
    if (class_prefix_matches(rest, "KT")) return 110;
    if (class_prefix_matches(rest, "CSHT")) return 111;
    if (class_prefix_matches(rest, "X2") || class_prefix_matches(rest, "THXD") || class_prefix_matches(rest, "DTTM")) return 117;
    if (class_prefix_matches(rest, "MT") || class_prefix_matches(rest, "QLMT")) return 118;
    if (class_prefix_matches(rest, "KX") || class_prefix_matches(rest, "QLCN")) return 121;
    if (class_prefix_matches(rest, "ES") || class_prefix_matches(rest, "ECE")) return 122;
    if (class_prefix_matches(rest, "PFIEV")) return 123;
    return 0;
}

static int str_casecmp(const char *a, const char *b) {
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int class_name_has_section(const char *className)
{
    size_t len = strlen(className);
    if (len < 2) return 0;
    char last = className[len - 1];
    return last >= '1' && last <= '9';
}

void get_class_group_prefix(const char *className, char *prefix_out)
{
    size_t len = strlen(className);
    if (class_name_has_section(className) && len > 0) {
        strncpy(prefix_out, className, len - 1);
        prefix_out[len - 1] = '\0';
    } else {
        strcpy(prefix_out, className);
    }
}

int count_students_in_class(const struct SinhVien ArrSinhVien[], int slsv, const char *className)
{
    int count = 0;
    char class_filter[64] = "";
    if (className != NULL && strlen(className) > 0) {
        strncpy(class_filter, className, sizeof(class_filter) - 1);
        class_filter[sizeof(class_filter) - 1] = '\0';
        for (char *p = class_filter; *p; p++) {
            *p = toupper((unsigned char)*p);
        }
    }
    for (int i = 0; i < slsv; i++) {
        if (strcmp(ArrSinhVien[i].Class, class_filter) == 0) {
            count++;
        }
    }
    return count;
}

void build_class_limit_message(const struct SinhVien ArrSinhVien[], int slsv, const char *className, char *out_msg, size_t out_len)
{
    char prefix[128];
    get_class_group_prefix(className, prefix);
    char classes[10][32] = {{0}};
    int counts[10] = {0};
    int classCount = 0;
    int maxSection = 0;
    int has_open = 0;

    for (int i = 0; i < slsv; i++) {
        const char *currentClass = ArrSinhVien[i].Class;
        if (!class_name_has_section(currentClass)) continue;

        char currentPrefix[128];
        get_class_group_prefix(currentClass, currentPrefix);
        if (strcmp(currentPrefix, prefix) != 0) continue;

        int found = 0;
        for (int j = 0; j < classCount; j++) {
            if (strcmp(classes[j], currentClass) == 0) {
                counts[j]++;
                found = 1;
                break;
            }
        }
        if (!found && classCount < 10) {
            strncpy(classes[classCount], currentClass, sizeof(classes[classCount]) - 1);
            classes[classCount][sizeof(classes[classCount]) - 1] = '\0';
            counts[classCount] = 1;
            classCount++;
        }

        int section = currentClass[strlen(currentClass) - 1] - '0';
        if (section > maxSection) maxSection = section;
    }

    int existingCount = count_students_in_class(ArrSinhVien, slsv, className);
    int nextSection = (maxSection < 9 ? maxSection + 1 : 0);
    char buffer[512];
    int len = snprintf(buffer, sizeof(buffer), "⚠️ Lớp %s đã đầy (%d sinh viên). ", className, existingCount);

    if (classCount > 0) {
        len += snprintf(buffer + len, sizeof(buffer) - len, "Các lớp cùng ngành: ");
        for (int i = 0; i < classCount && len < (int)sizeof(buffer); i++) {
            len += snprintf(buffer + len, sizeof(buffer) - len, "%s (%d)%s", classes[i], counts[i], (i < classCount - 1) ? ", " : "");
            if (counts[i] < 100) has_open = 1;
        }
    }

    if (has_open) {
        snprintf(buffer + len, sizeof(buffer) - len, ". Hãy thêm sinh viên vào lớp còn trống (< 100) hoặc tạo lớp mới.");
    } else if (nextSection != 0) {
        char nextClass[64];
        snprintf(nextClass, sizeof(nextClass), "%s%d", prefix, nextSection);
        snprintf(buffer + len, sizeof(buffer) - len, ". Hãy tạo lớp mới: %s.", nextClass);
    } else {
        snprintf(buffer + len, sizeof(buffer) - len, ". Đã đạt tối đa 9 lớp cùng ngành.");
    }

    strncpy(out_msg, buffer, out_len - 1);
    out_msg[out_len - 1] = '\0';
}

int is_valid_export_filename(const char *filename)
{
    if (filename == NULL) return 0;
    size_t len = strlen(filename);
    if (len <= 4) return 0;
    if (str_casecmp(filename + len - 4, ".txt") != 0) return 0;

    for (const char *p = filename; *p; p++) {
        if (*p == '/' || *p == '\\' || *p == ':' || *p == '*' || *p == '?' || *p == '"' || *p == '<' || *p == '>' || *p == '|' || *p == '\n' || *p == '\r' || *p == '\t') {
            return 0;
        }
    }
    return 1;
}

int export_student_list_to_file(const char *filename, const char *classFilter, const struct SinhVien ArrSinhVien[], int slsv)
{
    if (filename == NULL || strlen(filename) == 0) return 0;

    FILE *f = fopen(filename, "w");
    if (f == NULL) return 0;

    int exported = 0;
    char filter[64] = "";
    if (classFilter != NULL && strlen(classFilter) > 0) {
        strncpy(filter, classFilter, sizeof(filter) - 1);
        filter[sizeof(filter) - 1] = '\0';
        for (char *p = filter; *p; p++) {
            *p = toupper((unsigned char)*p);
        }
    }

    for (int i = 0; i < slsv; i++) {
        if (strlen(filter) > 0 && strcmp(ArrSinhVien[i].Class, filter) != 0) continue;
        fprintf(f, "%lld|%s|%s|%02d/%02d/%04d|%s|%s|%s\n",
                ArrSinhVien[i].MaSV,
                ArrSinhVien[i].Name,
                ArrSinhVien[i].Class,
                ArrSinhVien[i].NgaySinh.day,
                ArrSinhVien[i].NgaySinh.month,
                ArrSinhVien[i].NgaySinh.year,
                ArrSinhVien[i].Gender,
                ArrSinhVien[i].Address,
                ArrSinhVien[i].Email);
        exported++;
    }

    fclose(f);
    return exported > 0;
}

int LuuVaoFile(struct SinhVien ArrSinhVien[], int slsv)
{
    if (slsv == 0 || strlen(CurrentFileName) == 0) return 0;
    FILE *f = fopen(CurrentFileName, "w");
    if (f == NULL)
    {
        printf("Loi! Khong the ghi vao file %s.\n", CurrentFileName);
        return 0;
    }
    for (int i = 0; i < slsv; i++)
    {
        fprintf(f, "%lld|%s|%s|%d|%d|%d|%s|%s|%s\n",
                ArrSinhVien[i].MaSV, ArrSinhVien[i].Name, ArrSinhVien[i].Class,
                ArrSinhVien[i].NgaySinh.day, ArrSinhVien[i].NgaySinh.month, ArrSinhVien[i].NgaySinh.year,
                ArrSinhVien[i].Gender, ArrSinhVien[i].Address, ArrSinhVien[i].Email);
    }
    fclose(f);
    setColor(10);
    printf("\nDa cap nhat moi thong tin vao file: %s\n", CurrentFileName);
    return 1;
}

int  NhapTuFile(struct SinhVien ArrSinhVien[], int *slsv, const char *fileName)
{
    if (strlen(CurrentFileName) > 0) printf("[LAST FILE: %s]\n", CurrentFileName);
    FILE *f = fopen(fileName, "r");
    if (f == NULL) {
        setColor(12);
        return 0;
    }

    *slsv = 0;
    strcpy(CurrentFileName, fileName);
    save_last_filename();

    char line[500];
    while (fgets(line, sizeof(line), f) && *slsv < 1000)
    {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) < 5) continue;

        char *token = strtok(line, "|");
        if (token) {
            ArrSinhVien[*slsv].MaSV = atoll(token);

            token = strtok(NULL, "|");
            if (token)
            {
                strcpy(ArrSinhVien[*slsv].Name, token);
                vietHoaTatCa(ArrSinhVien[*slsv].Name);
            }
            token = strtok(NULL, "|");
            if (token) strcpy(ArrSinhVien[*slsv].Class, token);

            token = strtok(NULL, "|");
            if (token) ArrSinhVien[*slsv].NgaySinh.day = atoi(token);

            token = strtok(NULL, "|");
            if (token) ArrSinhVien[*slsv].NgaySinh.month = atoi(token);

            token = strtok(NULL, "|");
            if (token) ArrSinhVien[*slsv].NgaySinh.year = atoi(token);

            token = strtok(NULL, "|");
            if (token) strcpy(ArrSinhVien[*slsv].Gender, token);

            token = strtok(NULL, "|");
            if (token) strcpy(ArrSinhVien[*slsv].Address, token);

            token = strtok(NULL, "|");
            if (token) strcpy(ArrSinhVien[*slsv].Email, token);
            if (ArrSinhVien[*slsv].MaSV > 0)
                ArrSinhVien[*slsv].MaKhoa = (int)(ArrSinhVien[*slsv].MaSV / 1000000);
            else
                ArrSinhVien[*slsv].MaKhoa = layMaKhoaTuLop(ArrSinhVien[*slsv].Class);

            (*slsv)++;
        }
    }
    fclose(f);
    setColor(10);
    printf("\n=> Da mo va nạp %d sinh vien tu file %s.\n", *slsv, CurrentFileName);
    return 1;
}

static void trim_whitespace(char *out, const char *s)
{
    const char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char *end = s + strlen(s);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    size_t len = end - start;
    if (len > 0) {
        memmove(out, start, len);
    }
    out[len] = '\0';
}

void XuatDanhSachSinhVien(const struct SinhVien ArrSinhVien[], int slsv)
{
    if (slsv == 0) {
        printf("\nDanh sach sinh vien trong, khong co gi de xuat.\n");
        return;
    }

    char filename[256];
    char classFilter[64] = "";

    xoaBoNhoDem();
    printf("\nNhap ten file xuat (VD: export.txt): ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) return;
    trim_whitespace(filename, filename);
    if (!is_valid_export_filename(filename)) {
        printf("\nTen file khong hop le. Vui long dung cuoi .txt va khong chua ky tu dac biet.\n");
        return;
    }

    printf("Nhap lop can loc (bo trong neu khong loc): ");
    if (fgets(classFilter, sizeof(classFilter), stdin) != NULL) {
        trim_whitespace(classFilter, classFilter);
        vietHoaTatCa(classFilter);
    }

    if (export_student_list_to_file(filename, classFilter, ArrSinhVien, slsv)) {
        printf("\nXuat file thanh cong: %s\n", filename);
        if (strlen(classFilter) > 0) printf("Danh sach da loc theo lop: %s\n", classFilter);
    } else {
        printf("\nXuat file that bai. Kiem tra lai ten file va quyen ghi.\n");
    }
}

void SuaSinhVien(struct SinhVien ArrSinhVien[], int slsv)
{
    if (slsv == 0) {
        printf("\nDanh sach sinh vien trong, khong co gi de sua.\n");
        return;
    }

    long long mssv;
    printf("\nNhap MSSV cua sinh vien can sua: ");
    if (scanf("%lld", &mssv) != 1) {
        printf("\nMSSV khong hop le.\n");
        xoaBoNhoDem();
        return;
    }
    xoaBoNhoDem();

    int idx = -1;
    for (int i = 0; i < slsv; i++) {
        if (ArrSinhVien[i].MaSV == mssv) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        printf("\nKhong tim thay sinh vien co MSSV %lld.\n", mssv);
        return;
    }

    printf("\nThong tin hien tai:\n");
    printf("  Ten: %s\n", ArrSinhVien[idx].Name);
    printf("  Ngay sinh: %02d/%02d/%04d\n", ArrSinhVien[idx].NgaySinh.day, ArrSinhVien[idx].NgaySinh.month, ArrSinhVien[idx].NgaySinh.year);
    printf("  Gioi tinh: %s\n", ArrSinhVien[idx].Gender);
    printf("  Dia chi: %s\n", ArrSinhVien[idx].Address);
    printf("  Lop: %s\n", ArrSinhVien[idx].Class);
    printf("  Email: %s\n", ArrSinhVien[idx].Email);

    char name[50] = "";
    char date[32] = "";
    char gender[16] = "";
    char address[150] = "";
    char classInput[64] = "";
    char email[50] = "";

    printf("\nNhap ten moi (bo trong de giu nguyen): ");
    if (fgets(name, sizeof(name), stdin) == NULL) return;
    trim_whitespace(name, name);

    printf("Nhap ngay sinh moi (DD/MM/YYYY) (bo trong de giu nguyen): ");
    if (fgets(date, sizeof(date), stdin) == NULL) return;
    trim_whitespace(date, date);

    printf("Nhap gioi tinh moi (Nam/Nu) (bo trong de giu nguyen): ");
    if (fgets(gender, sizeof(gender), stdin) == NULL) return;
    trim_whitespace(gender, gender);

    printf("Nhap dia chi moi (bo trong de giu nguyen): ");
    if (fgets(address, sizeof(address), stdin) == NULL) return;
    trim_whitespace(address, address);

    printf("Nhap lop moi (bo trong de giu nguyen): ");
    if (fgets(classInput, sizeof(classInput), stdin) == NULL) return;
    trim_whitespace(classInput, classInput);

    printf("Nhap email moi (bo trong de giu nguyen): ");
    if (fgets(email, sizeof(email), stdin) == NULL) return;
    trim_whitespace(email, email);

    if (strlen(name) > 0) {
        strncpy(ArrSinhVien[idx].Name, name, sizeof(ArrSinhVien[idx].Name) - 1);
        ArrSinhVien[idx].Name[sizeof(ArrSinhVien[idx].Name) - 1] = '\0';
        vietHoaTatCa(ArrSinhVien[idx].Name);
    }

    int d = ArrSinhVien[idx].NgaySinh.day;
    int m = ArrSinhVien[idx].NgaySinh.month;
    int y = ArrSinhVien[idx].NgaySinh.year;
    if (strlen(date) > 0) {
        if (sscanf(date, "%d/%d/%d", &d, &m, &y) != 3 || checkDate(d, m, y) != 1) {
            printf("\nNgay sinh khong hop le. Sua that bai.\n");
            return;
        }
    }
    ArrSinhVien[idx].NgaySinh.day = d;
    ArrSinhVien[idx].NgaySinh.month = m;
    ArrSinhVien[idx].NgaySinh.year = y;

    if (strlen(gender) > 0) {
        strncpy(ArrSinhVien[idx].Gender, gender, sizeof(ArrSinhVien[idx].Gender) - 1);
        ArrSinhVien[idx].Gender[sizeof(ArrSinhVien[idx].Gender) - 1] = '\0';
        vietHoaChuDau(ArrSinhVien[idx].Gender);
    }

    if (strlen(address) > 0) {
        strncpy(ArrSinhVien[idx].Address, address, sizeof(ArrSinhVien[idx].Address) - 1);
        ArrSinhVien[idx].Address[sizeof(ArrSinhVien[idx].Address) - 1] = '\0';
        vietHoaChuDau(ArrSinhVien[idx].Address);
    }

    if (strlen(email) > 0) {
        strncpy(ArrSinhVien[idx].Email, email, sizeof(ArrSinhVien[idx].Email) - 1);
        ArrSinhVien[idx].Email[sizeof(ArrSinhVien[idx].Email) - 1] = '\0';
    }

    if (strlen(classInput) > 0) {
        char classUpper[128] = "";
        char KhoaNew[128] = {0}, TenLopNew[128] = {0};
        char KhoaOld[128] = {0}, TenLopOld[128] = {0};

        trim_whitespace(classUpper, classInput);
        vietHoaTatCa(classUpper);

        TachLop(classUpper, KhoaNew, TenLopNew);
        TachLop(ArrSinhVien[idx].Class, KhoaOld, TenLopOld);

        if (ArrSinhVien[idx].MaSV == 0) {
            if (strlen(classUpper) == 0 || layMaKhoaTuLop(classUpper) == 0) {
                printf("\nLop moi khong hop le. Sua that bai.\n");
                return;
            }
            if (!class_name_has_section(classUpper)) {
                printf("\nLop moi phai ket thuc bang chu so 1-9. Sua that bai.\n");
                return;
            }
            int classSize = count_students_in_class(ArrSinhVien, slsv, classUpper);
            if (classSize >= 100) {
                char msg[512];
                build_class_limit_message(ArrSinhVien, slsv, classUpper, msg, sizeof(msg));
                printf("\n%s\n", msg);
                return;
            }
        } else {
            if (strcmp(KhoaNew, KhoaOld) != 0) {
                printf("\nMa khoa cua lop moi phai trung voi ma khoa cu. Sua that bai.\n");
                return;
            }
            int newMaKhoa = layMaKhoaTuLop(TenLopNew);
            if (newMaKhoa == 0 || newMaKhoa != ArrSinhVien[idx].MaKhoa) {
                printf("\nLop moi khong thuoc ma khoa cu cua sinh vien. Sua that bai.\n");
                return;
            }
            if (!class_name_has_section(classUpper)) {
                printf("\nLop moi phai ket thuc bang chu so 1-9. Sua that bai.\n");
                return;
            }
        }

        strncpy(ArrSinhVien[idx].Class, classUpper, sizeof(ArrSinhVien[idx].Class) - 1);
        ArrSinhVien[idx].Class[sizeof(ArrSinhVien[idx].Class) - 1] = '\0';
    }

    LuuVaoFile(ArrSinhVien, slsv);
    printf("\nSua thong tin sinh vien thanh cong!\n");
}

const char* layTen(const char* hoTen){
    const char* ten = strrchr(hoTen, ' ');

    if (ten == NULL)
        return hoTen;

    return ten + 1;
}
void TachTen(const char *hoVaTen, char *hoVaTenDem, char *ten) {
    strcpy(hoVaTenDem, hoVaTen);
    char *lastSpace = strrchr(hoVaTenDem, ' ');
    if (lastSpace != NULL) {
        strcpy(ten, lastSpace + 1);
        *lastSpace = '\0';
    } else {
        strcpy(ten, hoVaTenDem);
        hoVaTenDem[0] = '\0';
    }
}

int SoSanhTenViet(const char *name1, const char *name2) {
    char hoVaTenDem1[50], ten1[20];
    char hoVaTenDem2[50], ten2[20];

    TachTen(name1, hoVaTenDem1, ten1);
    TachTen(name2, hoVaTenDem2, ten2);

    int cmp = strcmp(ten1, ten2);
    if (cmp != 0) {
        return cmp;
    }

    return strcmp(hoVaTenDem1, hoVaTenDem2);
}

int checkDate(int day, int month, int year){
    if (1990 > year || year > 2008) return -1;

    int maxDay;
    if (month == 2)
        maxDay = (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0)) ? 29 : 28;
    else if (month == 4 || month == 6 || month == 9 || month == 11)
        maxDay = 30;
    else if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
        maxDay = 31;
    else return -2;

    if (1 > day || day > maxDay) return -3;
    return 1;
}

static int compare_class_codes(const char *a, const char *b)
{
    int a_prefix = 0;
    int b_prefix = 0;
    if (isdigit((unsigned char)a[0]) && isdigit((unsigned char)a[1])) {
        a_prefix = (a[0] - '0') * 10 + (a[1] - '0');
    }
    if (isdigit((unsigned char)b[0]) && isdigit((unsigned char)b[1])) {
        b_prefix = (b[0] - '0') * 10 + (b[1] - '0');
    }

    if (a_prefix != b_prefix) {
        return a_prefix - b_prefix;
    }

    const char *a_rest = a + 2;
    const char *b_rest = b + 2;
    return strcmp(a_rest, b_rest);
}

void SapXepTheoLop(struct SinhVien ArrSinhVien[], int slsv)
{
    if (slsv == 0) {
        setColor(12);
        printf("\nDanh sach trong, khong co gi de sap xep.\n");
        setColor(7);
        return;
    }
    for (int i = 0; i < slsv - 1; i++) {
        for (int j = i + 1; j < slsv; j++) {
            if (compare_class_codes(ArrSinhVien[i].Class, ArrSinhVien[j].Class) > 0) {
                struct SinhVien temp = ArrSinhVien[i];
                ArrSinhVien[i] = ArrSinhVien[j];
                ArrSinhVien[j] = temp;
            }
        }
    }
    DaSapXep = 1;
    setColor(10);
    printf("\nDa sap xep sinh vien theo lop.\n");
    setColor(7);
}

void SapXepTheoTen(struct SinhVien ArrSinhVien[], int slsv)
{
    if (slsv == 0)
    {
        setColor(12);
        printf("\nDanh sach trong!\n");
        return;
    }

    for (int i = 0; i < slsv - 1; i++)
    {
        for (int j = i + 1; j < slsv; j++)
        {
            if (SoSanhTenViet(ArrSinhVien[i].Name, ArrSinhVien[j].Name) > 0)
            {
                struct SinhVien temp = ArrSinhVien[i];
                ArrSinhVien[i] = ArrSinhVien[j];
                ArrSinhVien[j] = temp;
            }
        }
    }

    DaSapXep = 1;
    setColor(10);
    printf("\n=> Da sap xep xong ten cho nhom sinh vien moi.\n");
}

void CapMaSoSinhVien(struct SinhVien ArrSinhVien[], int slsv)
{
    if (slsv == 0)
    {
        setColor(12);
        printf("\nDanh sach trong!\n");
        return;
    }

    if (DaSapXep == 0)
    {
        setColor(12);
        printf("\n[CANH BAO] Ban vua them sinh vien moi. Hay chon muc 5 de sap xep ten truoc khi cap ma!\n");
        return;
    }
    int sttKhoa[250] = {0};
    for (int i = 0; i < slsv; i++)
    {
        if (ArrSinhVien[i].MaSV != 0)
        {
            int currentSTT = ArrSinhVien[i].MaSV % 10000;
            int mk = ArrSinhVien[i].MaKhoa;
            if (currentSTT > sttKhoa[mk])
            {
                sttKhoa[mk] = currentSTT;
            }
        }
    }
    int count = 0;
    for (int i = 0; i < slsv; i++)
    {
        if (ArrSinhVien[i].MaSV == 0)
        {
            int namHoc = 0;
            sscanf(ArrSinhVien[i].Class, "%2d", &namHoc);
            int maKhoa = layMaKhoaTuLop(ArrSinhVien[i].Class);

            if (maKhoa != 0)
            {
                sttKhoa[maKhoa]++;
                ArrSinhVien[i].MaKhoa = maKhoa;
                ArrSinhVien[i].MaSV = (long long)maKhoa * 1000000 + (long long)namHoc * 10000 + sttKhoa[maKhoa];
                count++;
            }
        }
    }
    if (count > 0)
    {
        setColor(10);
        printf("\n=> Da cap MSSV cho %d sinh vien moi.\n", count);
    }
    else
    {
        setColor(12);
        printf("\nKhong co sinh vien moi nao can cap ma.\n");
    }
}

void CapEmailSinhVien(struct SinhVien ArrSinhVien[], int slsv)
{
    if (slsv == 0)
    {
        printf("\nDanh sach trong!\n");
        return;
    }
    int count = 0;
    for (int i = 0; i < slsv; i++)
    {
        if (ArrSinhVien[i].MaSV != 0)
        {
            sprintf(ArrSinhVien[i].Email, "%lld@sv.dut.udn.vn", ArrSinhVien[i].MaSV);
            count++;
        }
    }

    if (count > 0)
    {
        setColor(10);
        printf("\n=> Da cap email thanh cong cho %d sinh vien co MSSV.\n", count);
    }
    else
    {
        setColor(12);
        printf("\nChua co sinh vien nao duoc cap MSSV de khoi tao email.\n");
    }
}

void SapXepTheoMSSV(struct SinhVien arr[], int count)
{
    struct SinhVien sortedWithMSSV[1000];
    struct SinhVien withoutMSSV[1000];
    int withCount = 0;
    int withoutCount = 0;

    for (int i = 0; i < count; i++) {
        if (arr[i].MaSV != 0) {
            sortedWithMSSV[withCount++] = arr[i];
        } else {
            withoutMSSV[withoutCount++] = arr[i];
        }
    }

    for (int i = 0; i < withCount - 1; i++) {
        for (int j = i + 1; j < withCount; j++) {
            if (sortedWithMSSV[i].MaSV > sortedWithMSSV[j].MaSV) {
                struct SinhVien temp = sortedWithMSSV[i];
                sortedWithMSSV[i] = sortedWithMSSV[j];
                sortedWithMSSV[j] = temp;
            }
        }
    }

    int idx = 0;
    for (int i = 0; i < withCount; i++) {
        arr[idx++] = sortedWithMSSV[i];
    }
    for (int i = 0; i < withoutCount; i++) {
        arr[idx++] = withoutMSSV[i];
    }
}

void SapXepTheoNgaySinh(struct SinhVien arr[], int count)
{
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (arr[i].NgaySinh.year > arr[j].NgaySinh.year ||
                (arr[i].NgaySinh.year == arr[j].NgaySinh.year && arr[i].NgaySinh.month > arr[j].NgaySinh.month) ||
                (arr[i].NgaySinh.year == arr[j].NgaySinh.year && arr[i].NgaySinh.month == arr[j].NgaySinh.month && arr[i].NgaySinh.day > arr[j].NgaySinh.day)) {
                struct SinhVien temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
            }
        }
    }
}

void sort_students_by_name(struct SinhVien arr[], int count)
{
    for (int i = 0; i < count - 1; i++) {
        int min_index = i;
        for (int j = i + 1; j < count; j++) {
            if (strcmp(arr[j].Name, arr[min_index].Name) < 0) {
                min_index = j;
            }
        }
        if (min_index != i) {
            struct SinhVien temp = arr[i];
            arr[i] = arr[min_index];
            arr[min_index] = temp;
        }
    }
}

void sort_students_smart(struct SinhVien arr[], int count)
{
    struct SinhVien tempUnsorted[1000];
    int unsortedCount = 0;

    for (int i = 0; i < count; i++) {
        if (arr[i].MaSV == 0) {
            tempUnsorted[unsortedCount++] = arr[i];
        }
    }

    for (int i = 0; i < unsortedCount - 1; i++) {
        int min_index = i;
        for (int j = i + 1; j < unsortedCount; j++) {
            if (SoSanhTenViet(tempUnsorted[j].Name, tempUnsorted[min_index].Name) < 0) {
                min_index = j;
            }
        }
        if (min_index != i) {
            struct SinhVien temp = tempUnsorted[i];
            tempUnsorted[i] = tempUnsorted[min_index];
            tempUnsorted[min_index] = temp;
        }
    }

    struct SinhVien newArr[1000];
    int newIdx = 0;

    for (int i = 0; i < count; i++) {
        if (arr[i].MaSV != 0) {
            newArr[newIdx++] = arr[i];
        }
    }

    for (int i = 0; i < unsortedCount; i++) {
        newArr[newIdx++] = tempUnsorted[i];
    }

    for (int i = 0; i < count; i++) {
        arr[i] = newArr[i];
    }
}

void ThemSinhVien(struct SinhVien ArrSinhVien[], int *slsv)
{
    printf("\n--- Nhap thong tin sinh vien thu %d ---\n", *slsv + 1);
    printf("Nhap Ho va Ten: ");
    scanf(" %[^\n]", ArrSinhVien[*slsv].Name);
    vietHoaTatCa(ArrSinhVien[*slsv].Name);
    printf("Nhap Ngay Thang Nam sinh (DD MM YYYY): ");
    while (scanf("%d %d %d", &ArrSinhVien[*slsv].NgaySinh.day, &ArrSinhVien[*slsv].NgaySinh.month, &ArrSinhVien[*slsv].NgaySinh.year) != 3
    || checkDate(ArrSinhVien[*slsv].NgaySinh.day, ArrSinhVien[*slsv].NgaySinh.month, ArrSinhVien[*slsv].NgaySinh.year) != 1)
    {
        setColor(12);
        printf("Ngay thang khong hop le! Moi nhap lai (DD MM YYYY): ");
        xoaBoNhoDem();
        setColor(7);
    }
    printf("Nhap Gioi Tinh (Nam / Nu): ");
    scanf(" %[^\n]", ArrSinhVien[*slsv].Gender);
    vietHoaChuDau(ArrSinhVien[*slsv].Gender);
    printf("Nhap Dia Chi: ");
    scanf(" %[^\n]", ArrSinhVien[*slsv].Address);
    vietHoaChuDau(ArrSinhVien[*slsv].Address);
    printf("Nhap Lop: ");
    while (1){
        scanf(" %[^\n]", ArrSinhVien[*slsv].Class);
        vietHoaTatCa(ArrSinhVien[*slsv].Class);
        if (layMaKhoaTuLop(ArrSinhVien[*slsv].Class) != 0) break;
        setColor(12);
        printf("Lop khong hop le, khong ton tai trong chuong trinh! Moi nhap lai: ");
        setColor(7);
    }

    ArrSinhVien[*slsv].MaSV = 0;
    ArrSinhVien[*slsv].MaKhoa = 0;
    strcpy(ArrSinhVien[*slsv].Email, "Chua co");
    DaSapXep = 0;
    (*slsv)++;
    printf("=> Da them sinh vien thanh cong!\n");

}

void TimKiemSinhVien(struct SinhVien ArrSinhVien[], int slsv)
{
    if (slsv == 0)
    {
        setColor(12);
        printf("\nDanh sach trong, khong the tim kiem!\n");
        return;
    }
    int luaChon;
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║          TIM KIEM SINH VIEN                ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║ 1. Tim theo Ten                            ║\n");
    printf("║ 2. Tim theo Ma so sinh vien (MSSV)         ║\n");
    printf("║ 0. Quay lai                                ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    luaChon = nhapSoNguyen();
    int timThay = 0;
    if (luaChon == 1)
    {
        char tenTim[50];
        printf("Nhap ten sinh vien can tim: ");
        scanf(" %[^\n]", tenTim);
        vietHoaTatCa(tenTim);
        printf("\nKet qua tim kiem cho ten \"%s\":\n", tenTim);
        for (int i = 0; i < slsv; i++) {
            if (strstr(ArrSinhVien[i].Name, tenTim) != NULL)
            {
                printf("- %-12lld | %-20s | Lop: %s\n", ArrSinhVien[i].MaSV, ArrSinhVien[i].Name, ArrSinhVien[i].Class);
                timThay = 1;
            }
        }
    }
    else if (luaChon == 2)
    {
        long long maTim;
        printf("Nhap MSSV can tim: ");
        while (scanf("%lld", &maTim) != 1)
        {
            setColor(12);
            printf("Loi! Nhap lai MSSV: ");
            xoaBoNhoDem();
            setColor(7);
        }

        for (int i = 0; i < slsv; i++)
        {
            if (ArrSinhVien[i].MaSV == maTim)
            {
                setColor(10);
                printf("\nDa tim thay sinh vien:\n");
                printf("Ho ten: %s\nLop: %s\nNgay sinh: %02d/%02d/%d\n",
                    ArrSinhVien[i].Name, ArrSinhVien[i].Class,
                    ArrSinhVien[i].NgaySinh.day, ArrSinhVien[i].NgaySinh.month, ArrSinhVien[i].NgaySinh.year);
                timThay = 1;
                break;
            }
        }
    }
    else
    {
        setColor(12);
        printf("\nLua chon khong hop le!\n");
        return;
    }

    if (!timThay)
    {
        setColor(12);
        printf("\nKhong tim thay sinh vien phu hop.\n");
        return;
    }
}

void XoaSinhVien(struct SinhVien ArrSinhVien[], int *slsv)
{
    if (*slsv == 0)
    {
        setColor(12);
        printf("\nDanh sach trong, khong co gi de xoa!\n");
        setColor(7);
        return;
    }
    int luaChon;
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║              XOA SINH VIEN                 ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║ 1. Xoa theo Ten                            ║\n");
    printf("║ 2. Xoa theo Ma so sinh vien (MSSV)         ║\n");
    printf("║ 0. Quay lai                                ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf(" Chon phuong thuc: ");
    luaChon = nhapSoNguyen();
    if (luaChon == 1)
    {
        char tenXoa[50];
        printf("Nhap ten can xoa: ");
        scanf(" %[^\n]", tenXoa);
        vietHoaTatCa(tenXoa);
        int indices[1000], count = 0;
        for (int i = 0; i < *slsv; i++)
        {
            if (strstr(ArrSinhVien[i].Name, tenXoa) != NULL)
            {
                indices[count++] = i;
            }
        }
        if (count == 0)
        {
            setColor(12);
            printf("\nKhong tim thay sinh vien nao co ten phu hop.\n");
            return;
        }
        printf("\nTim thay %d sinh vien phu hop:", count);
        printf("\n1. Xoa TAT CA sinh vien nay");
        printf("\n2. CHON sinh vien cu the de xoa");
        printf("\nChon: ");
        int kieuXoa = nhapSoNguyen();
        if (kieuXoa == 1)
        {
            for (int i = 0; i < *slsv; i++)
            {
                if (strstr(ArrSinhVien[i].Name, tenXoa) != NULL)
                {
                    for (int j = i; j < *slsv - 1; j++) ArrSinhVien[j] = ArrSinhVien[j+1];
                    (*slsv)--; i--;
                }
            }
            setColor(10);
            printf("\n=> Da xoa tat ca sinh vien co ten khop.\n");
        } else if (kieuXoa == 2)
        {
            printf("\nDanh sach sinh vien tim thay:\n");
            for (int i = 0; i < count; i++)
            {
                int idx = indices[i];
                printf("%d. %-20s | MSSV: %-12lld | Lop: %s\n", i + 1, ArrSinhVien[idx].Name, ArrSinhVien[idx].MaSV, ArrSinhVien[idx].Class);
            }
            printf("Chon STT muon xoa: ");
            int stt = nhapSoNguyen();
            if (stt >= 1 && stt <= count)
            {
                int target = indices[stt - 1];
                setColor(10);
                printf("\nDa xoa sinh vien: %s\n", ArrSinhVien[target].Name);
                for (int j = target; j < *slsv - 1; j++) ArrSinhVien[j] = ArrSinhVien[j+1];
                (*slsv)--;
            }
            else
            {
                setColor(12);
                printf("\nSTT khong hop le!\n");
            }
        }
    }
    else if (luaChon == 2)
    {
        long long maXoa;
        printf("Nhap MSSV can xoa: ");
        while (scanf("%lld", &maXoa) != 1)
        {
            setColor(12);
            printf("Loi! Nhap lai MSSV: ");
            xoaBoNhoDem();
        }
        int timThay = 0;
        for (int i = 0; i < *slsv; i++)
        {
            if (ArrSinhVien[i].MaSV == maXoa)
            {
                setColor(10);
                printf("\nDa xoa sinh vien: %s (MSSV: %lld)\n", ArrSinhVien[i].Name, ArrSinhVien[i].MaSV);
                for (int j = i; j < *slsv - 1; j++) ArrSinhVien[j] = ArrSinhVien[j+1];
                (*slsv)--;
                timThay = 1;
                break;
            }
        }
        if (!timThay){
            setColor(12);
            printf("\nKhong tim thay MSSV nay.\n");
        }
    }
}

void InDanhSachSinhVien(struct SinhVien ArrSinhVien[], int slsv)
{
    if (slsv == 0) {
        printf("\nDanh sach trong!\n");
        return;
    }
    printf("\n--- DANH SACH TAT CA SINH VIEN ---\n");

    int count = 0;
    int wName = strlen("TEN");
    int wClass = strlen("LOP");
    int wDate = 10;
    int wGender = strlen("GIOI TINH");
    int wAddress = strlen("DIA CHI");
    int wEmail = strlen("EMAIL");

    for (int i = 0; i < slsv; i++){
        if (strlen(ArrSinhVien[i].Name) > wName) wName = strlen(ArrSinhVien[i].Name);
        if (strlen(ArrSinhVien[i].Class) > wClass) wClass = strlen(ArrSinhVien[i].Class);
        if (strlen(ArrSinhVien[i].Address) > wAddress) wAddress = strlen(ArrSinhVien[i].Address);
        if (strlen(ArrSinhVien[i].Email) > wEmail) wEmail = strlen(ArrSinhVien[i].Email);
    }

    printf("╔════════════╦");
    for (int i = 0; i < wName+2; i++) printf("═");
    printf("╦");
    for (int i = 0; i < wClass+2; i++) printf("═");
    printf("╦");
    for (int i = 0; i < wDate+2; i++) printf("═");
    printf("╦");
    for (int i = 0; i < wGender+2; i++) printf("═");
    printf("╦");
    for (int i = 0; i < wAddress+2; i++) printf("═");
    printf("╦");
    for (int i = 0; i < wEmail+2; i++) printf("═");
    printf("╗\n");

    printf("║ %-10s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║\n",
        "MSSV",
        wName, "TEN",
        wClass, "LOP",
        wDate ,"NGAY SINH",
        wGender, "GIOI TINH",
        wAddress, "DIA CHI",
        wEmail, "EMAIL"
    );

    printf("╠════════════╬");
    for (int i = 0; i < wName+2; i++) printf("═");
    printf("╬");
    for (int i = 0; i < wClass+2; i++) printf("═");
    printf("╬");
    for (int i = 0; i < wDate+2; i++) printf("═");
    printf("╬");
    for (int i = 0; i < wGender+2; i++) printf("═");
    printf("╬");
    for (int i = 0; i < wAddress+2; i++) printf("═");
    printf("╬");
    for (int i = 0; i < wEmail+2; i++) printf("═");
    printf("╣\n");

    for (int i = 0; i < slsv; i++)
    {
        printf("║ %-10lld ║ %-*.*s ║ %-*s ║ %02d/%02d/%-4d ║ %-*s ║ %-*.*s ║ %-*.*s ║\n",
            ArrSinhVien[i].MaSV,
            wName, wName, ArrSinhVien[i].Name,
            wClass, ArrSinhVien[i].Class,
            ArrSinhVien[i].NgaySinh.day,
            ArrSinhVien[i].NgaySinh.month,
            ArrSinhVien[i].NgaySinh.year,
            wGender ,ArrSinhVien[i].Gender,
            wAddress, wAddress,ArrSinhVien[i].Address,
            wEmail ,wEmail ,ArrSinhVien[i].Email);
        count++;
    }

    printf("╚════════════╩");
    for (int i = 0; i < wName+2; i++) printf("═");
    printf("╩");
    for (int i = 0; i < wClass+2; i++) printf("═");
    printf("╩");
    for (int i = 0; i < wDate+2; i++) printf("═");
    printf("╩");
    for (int i = 0; i < wGender+2; i++) printf("═");
    printf("╩");
    for (int i = 0; i < wAddress+2; i++) printf("═");
    printf("╩");
    for (int i = 0; i < wEmail+2; i++) printf("═");
    printf("╝\n");

    if (count == 0) printf("=> Khong tim thay sinh vien nao\n");
}

void InDanhSachSinhVienTheoLop(struct SinhVien ArrSinhVien[], int slsv, char Class[30])
{
    int count = 0;
    int wName = strlen("TEN");
    int wDate = 10;
    int wGender = strlen("GIOI TINH");
    int wAddress = strlen("DIA CHI");
    int wEmail = strlen("EMAIL");

    for (int i = 0; i < slsv; i++){
        if (strlen(ArrSinhVien[i].Name) > wName) wName = strlen(ArrSinhVien[i].Name);
        if (strlen(ArrSinhVien[i].Address) > wAddress) wAddress = strlen(ArrSinhVien[i].Address);
        if (strlen(ArrSinhVien[i].Email) > wEmail) wEmail = strlen(ArrSinhVien[i].Email);
    }

    printf("╔════════════╦");
    for (int i = 0; i < wName+2; i++) printf("═");
    printf("╦");
    for (int i = 0; i < wDate+2; i++) printf("═");
    printf("╦");
    for (int i = 0; i < wGender+2; i++) printf("═");
    printf("╦");
    for (int i = 0; i < wAddress+2; i++) printf("═");
    printf("╦");
    for (int i = 0; i < wEmail+2; i++) printf("═");
    printf("╗\n");

    printf("║ %-10s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║\n",
        "MSSV",
        wName, "TEN",
        wDate ,"NGAY SINH",
        wGender, "GIOI TINH",
        wAddress, "DIA CHI",
        wEmail, "EMAIL"
    );

    printf("╠════════════╬");
    for (int i = 0; i < wName+2; i++) printf("═");
    printf("╬");
    for (int i = 0; i < wDate+2; i++) printf("═");
    printf("╬");
    for (int i = 0; i < wGender+2; i++) printf("═");
    printf("╬");
    for (int i = 0; i < wAddress+2; i++) printf("═");
    printf("╬");
    for (int i = 0; i < wEmail+2; i++) printf("═");
    printf("╣\n");

    for (int i = 0; i < slsv; i++)
    {
        if (strcmp(ArrSinhVien[i].Class, Class) != 0) continue;
        printf("║ %-10lld ║ %-*.*s ║ %02d/%02d/%-4d ║ %-*s ║ %-*.*s ║ %-*.*s ║\n",
            ArrSinhVien[i].MaSV,
            wName, wName, ArrSinhVien[i].Name,
            ArrSinhVien[i].NgaySinh.day,
            ArrSinhVien[i].NgaySinh.month,
            ArrSinhVien[i].NgaySinh.year,
            wGender ,ArrSinhVien[i].Gender,
            wAddress, wAddress,ArrSinhVien[i].Address,
            wEmail ,wEmail ,ArrSinhVien[i].Email);
        count++;
    }

    printf("╚════════════╩");
    for (int i = 0; i < wName+2; i++) printf("═");
    printf("╩");
    for (int i = 0; i < wDate+2; i++) printf("═");
    printf("╩");
    for (int i = 0; i < wGender+2; i++) printf("═");
    printf("╩");
    for (int i = 0; i < wAddress+2; i++) printf("═");
    printf("╩");
    for (int i = 0; i < wEmail+2; i++) printf("═");
    printf("╝\n");

    if (count == 0) printf("=> Khong tim thay sinh vien nao thuoc lop %s\n", Class);
}

void Menutitle()
{
    clearScreen();
    printf("\n\t%c DO AN LAP TRINH TINH TOAN %c\n", 36, 36);
    printf("%c THUC HIEN BOI: TRAN KHANH DUY & LA TRUNG THUC %c\n", 36, 36);
    setColor(11);

    printf("╔══════════════════════════════════════╗\n");
    printf("║     HE THONG QUAN LY SINH VIEN       ║\n");
    printf("╠══════════════════════════════════════╣\n");
    setColor(12);
    printf("║  0. Thoat chuong trinh               ║\n");
    setColor(10);
    printf("║  1. Quan ly sinh vien                ║\n");
    setColor(11);
    printf("╚══════════════════════════════════════╝\n");

    setColor(7);
}

void MenuQuanLySinhVien()
{

    clearScreen();
    printf("\n\t%c DO AN LAP TRINH TINH TOAN %c\n", 36, 36);
    printf("%c THUC HIEN BOI: TRAN KHANH DUY & LA TRUNG THUC %c\n", 36, 36);
    setColor(11);
    setColor(14);

    printf("╔══════════════════════════════════════╗\n");
    printf("║        QUAN LY SINH VIEN             ║\n");
    printf("╠══════════════════════════════════════╣\n");
    setColor(12);
    printf("║  0. Quay lai                         ║\n");



    setColor(10);
    printf("║  1. Them sinh vien                   ║\n");
    printf("║  2. Sap xep                          ║\n");
    printf("║  3. Tim kiem                         ║\n");
    printf("║  4. Xoa sinh vien                    ║\n");
    printf("║  5. Cap MSSV                         ║\n");
    printf("║  6. Cap Email                        ║\n");
    printf("║  7. In danh sach                     ║\n");
    printf("║  8. Xuat danh sach ra file           ║\n");
    printf("║  9. Sua thong tin sinh vien         ║\n");
    setColor(14);
    printf("╚══════════════════════════════════════╝\n");

    setColor(7);
}

void MenuNhapSinhVien()
{
    printf("\n");
    printf("╔══════════════════════════════════════╗\n");
    printf("║         NHAP SINH VIEN               ║\n");
    printf("╠══════════════════════════════════════╣\n");
    printf("║ 0. Quay lai Menu Quan Ly             ║\n");
    printf("║ 1. Nhap sinh vien tu file            ║\n");
    printf("║ 2. Them sinh vien tu ban phim        ║\n");
    printf("╚══════════════════════════════════════╝\n");
}

void MenuInDanhSachSinhVien()
{
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║        IN DANH SACH SINH VIEN              ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║ 1. In danh sach tat ca                     ║\n");
    printf("║ 2. In danh sach theo lop                   ║\n");
    printf("║ 0. Quay lai Menu Quan Ly                   ║\n");
    printf("╚════════════════════════════════════════════╝\n");
}

/*
int main(void)
{
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);

    struct SinhVien ArrSinhVien[1000];
    int slsv = 0;
    int choice = -1;

    load_last_filename();
    if (NhapTuFile(ArrSinhVien, &slsv, CurrentFileName) == 0) {
        setColor(12);
        printf("\nKhong the mo file du lieu %s. Bat dau voi danh sach trong.\n", CurrentFileName);
        setColor(7);
    }

    while (choice != 0) {
        MenuQuanLySinhVien();
        printf("\nNhap lua chon: ");
        choice = nhapSoNguyen();

        switch (choice) {
            case 0:
                printf("\nThoat chuong trinh.\n");
                break;
            case 1: {
                int t = -1;
                MenuNhapSinhVien();
                printf("\nNhap lua chon: ");
                t = nhapSoNguyen();
                if (t == 1) {
                    char filename[256];
                    xoaBoNhoDem();
                    printf("\nNhap ten file de nap du lieu: ");
                    if (fgets(filename, sizeof(filename), stdin)) {
                        trim_whitespace(filename, filename);
                        if (strlen(filename) > 0) {
                            NhapTuFile(ArrSinhVien, &slsv, filename);
                        }
                    }
                } else if (t == 2) {
                    ThemSinhVien(ArrSinhVien, &slsv);
                    LuuVaoFile(ArrSinhVien, slsv);
                }
                break;
            }
            case 2:
                SapXepTheoLop(ArrSinhVien, slsv);
                break;
            case 3:
                TimKiemSinhVien(ArrSinhVien, slsv);
                break;
            case 4:
                XoaSinhVien(ArrSinhVien, &slsv);
                LuuVaoFile(ArrSinhVien, slsv);
                break;
            case 5:
                CapMaSoSinhVien(ArrSinhVien, slsv);
                LuuVaoFile(ArrSinhVien, slsv);
                break;
            case 6:
                CapEmailSinhVien(ArrSinhVien, slsv);
                LuuVaoFile(ArrSinhVien, slsv);
                break;
            case 7:
                InDanhSachSinhVien(ArrSinhVien, slsv);
                break;
            case 8:
                XuatDanhSachSinhVien(ArrSinhVien, slsv);
                break;
            case 9:
                SuaSinhVien(ArrSinhVien, slsv);
                break;
            default:
                setColor(12);
                printf("\nLua chon khong hop le. Vui long thu lai.\n");
                setColor(7);
                break;
        }
    }

    LuuVaoFile(ArrSinhVien, slsv);
    return 0;
}
*/