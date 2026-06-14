#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "quanlysinhvien.h"

// Biến toàn cục
struct SinhVien ArrSinhVien[1000];
int slsv = 0;
extern int DaSapXep;

typedef struct {
    GtkWidget *entryName;
    GtkWidget *entryDate;
    GtkWidget *entryGender;
    GtkWidget *entryAddress;
    GtkWidget *entryClass;

    GtkWidget *stack;
    GtkWidget *list_box;
    GtkWidget *msg_label; // Label riêng để hiện thông báo cho đẽp
    
    GtkWidget *searchEntryName;
    GtkWidget *searchEntryMSSV;
    GtkWidget *searchListBox;
    
    GtkWidget *deleteListBox;
    GtkWidget *deleteMsg;
    int selectedDeleteIndex;
    GtkWidget *deleteEntry;
    GtkWidget *deleteSearchListBox;
    GtkWidget *deleteSearchMsg;
    
    GtkWidget *sortCriteriaDropDown;
    GtkWidget *sortListBox;
    GtkWidget *sortMsg;

    GtkWidget *capListBox;
    GtkWidget *capMsg;

    GtkWidget *capEmailListBox;
    GtkWidget *capEmailMsg;

    GtkWidget *listFilterEntry;

    GtkWidget *fileEntry;
    GtkWidget *oldFileLabel;
    GtkWidget *fileMsgLabel; // Label riêng để hiện thông báo cho đẽp

    GtkWidget * previewText;
} SinhvienWidgets;

typedef struct {
    SinhvienWidgets *app;
    char name[100];
    long long mssv;
    int selectedIndex;
} DeleteData;

/// ======= FORWARD DECLARATIONS =======
static void on_delete_by_mssv_confirm(GObject *source, GAsyncResult *result, gpointer data);
static void on_delete_selected_confirm(GObject *source, GAsyncResult *result, gpointer data);
void refresh_delete_list(SinhvienWidgets *app);
static void trim_whitespace(char *dest, const char *src);
static void show_export_file_dialog(SinhvienWidgets *app);

typedef struct {
    SinhvienWidgets *app;
    GtkWidget *entry_file;
    GtkWidget *window;
} ExportPreviewData;

typedef struct {
    SinhvienWidgets *app;
    int index;
    GtkWidget *entryName;
    GtkWidget *entryDate;
    GtkWidget *entryGender;
    GtkWidget *entryAddress;
    GtkWidget *entryClass;
    GtkWidget *entryEmail;
    GtkWidget *window;
} EditDialogData;

static void on_edit_clicked(GtkWidget *widget, gpointer data);
static void on_edit_save(GtkWidget *widget, gpointer user_data);
static void on_reset_delete_view(GtkWidget *widget, gpointer data);

/// ======= HÀM HỖ TRỢ HIỂN THỊ THÔNG BÁO ======= 
void show_message(SinhvienWidgets *app, const char *msg, const char *color) {
    char *markup = g_strdup_printf("<span color='%s' weight='bold'>%s</span>", color, msg);
    gtk_label_set_markup(GTK_LABEL(app->msg_label), markup);
    g_free(markup);

    /* Also show a modal alert dialog so the user sees the message explicitly */
    GtkAlertDialog *alert = gtk_alert_dialog_new(msg);
    gtk_alert_dialog_set_detail(alert, "");
    gtk_alert_dialog_set_buttons(alert, (const char *[]) {"OK", NULL});
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_root(app->stack));
    if (parent) {
        gtk_alert_dialog_show(alert, parent);
    } else {
        gtk_alert_dialog_show(alert, NULL);
    }
}

static void trim_whitespace(char *dest, const char *src) {
    const char *start = src;
    const char *end = src + strlen(src);

    while (*start && g_ascii_isspace(*start)) {
        start++;
    }
    while (end > start && g_ascii_isspace(*(end - 1))) {
        end--;
    }

    size_t len = (size_t)(end - start);
    if (len > 0) {
        memcpy(dest, start, len);
    }
    dest[len] = '\0';
}

/// ======= HÀM HỖ TRỢ TẠO KHUNG ========
void set_margin_all(GtkWidget *GW, int n){
    if (GW == NULL) return ;

    gtk_widget_set_margin_top(GW, n);
    gtk_widget_set_margin_bottom(GW, n);
    gtk_widget_set_margin_start(GW, n);
    gtk_widget_set_margin_end(GW, n);
}

static GtkWidget *create_student_card_box(int spacing) {
    GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
    set_margin_all(row, 10);
    gtk_widget_add_css_class(row, "student-card");
    gtk_widget_set_size_request(row, 320, 110);
    return row;
}

static void append_repeat(GString *s, const char *ch, int n) {
    for(int i = 0; i < n; i++)
        g_string_append(s, ch);
}

static GString *build_student_table(const char *filter) {
    GString *content = g_string_new("");

    int wSTT = 3;
    int wMSSV = 10;
    int wName = g_utf8_strlen("HỌ TÊN", -1) + 2;
    int wClass = g_utf8_strlen("LỚP", -1) + 2;
    int wDate = 10;
    int wGender = g_utf8_strlen("GIỚI TÍNH", -1);
    int wEmail = g_utf8_strlen("EMAIL", -1);
    int wAddress = g_utf8_strlen("ĐỊA CHỈ", -1);

    int count = 0;

    for(int i = 0; i < slsv; i++) {

        if(strlen(filter) > 0 &&
           strcmp(ArrSinhVien[i].Class, filter) != 0)
            continue;

        count++;

        int len;

        len = g_utf8_strlen(ArrSinhVien[i].Name, -1);
        if(len > wName) wName = len;

        len = g_utf8_strlen(ArrSinhVien[i].Class, -1);
        if(len > wClass) wClass = len;

        len = g_utf8_strlen(ArrSinhVien[i].Gender, -1);
        if(len > wGender) wGender = len;

        len = g_utf8_strlen(ArrSinhVien[i].Email, -1);
        if(len > wEmail) wEmail = len;

        len = g_utf8_strlen(ArrSinhVien[i].Address, -1);
        if(len > wAddress) wAddress = len;
    }

    if(count == 0) {
        g_string_append(content,
            "Không có sinh viên phù hợp.\n");
        return content;
    }

    // ===== Đỉnh bảng =====

    g_string_append(content, "╔");

    append_repeat(content, "═", wSTT + 2);
    g_string_append(content, "╦");

    append_repeat(content, "═", wMSSV + 2);
    g_string_append(content, "╦");

    append_repeat(content, "═", wName + 2);
    g_string_append(content, "╦");

    append_repeat(content, "═", wClass + 2);
    g_string_append(content, "╦");

    append_repeat(content, "═", wGender + 2);
    g_string_append(content, "╦");

    append_repeat(content, "═", wDate + 2);
    g_string_append(content, "╦");

    append_repeat(content, "═", wEmail + 2);
    g_string_append(content, "╦");

    append_repeat(content, "═", wAddress + 2);

    g_string_append(content, "╗\n");

    // ===== Header =====

    g_string_append_printf(
        content,
        "║ %-*s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║\n",

        wSTT, "STT",
        wMSSV, "MSSV",
        wName, "HỌ TÊN",
        wClass, "LỚP",
        wGender, "GIỚI TÍNH",
        wDate, "NGÀY SINH",
        wEmail, "EMAIL",
        wAddress, "ĐỊA CHỈ"
    );

    // ===== Dòng phân cách =====

    g_string_append(content, "╠");

    append_repeat(content, "═", wSTT + 2);
    g_string_append(content, "╬");

    append_repeat(content, "═", wMSSV + 2);
    g_string_append(content, "╬");

    append_repeat(content, "═", wName + 2);
    g_string_append(content, "╬");

    append_repeat(content, "═", wClass + 2);
    g_string_append(content, "╬");

    append_repeat(content, "═", wGender + 2);
    g_string_append(content, "╬");

    append_repeat(content, "═", wDate + 2);
    g_string_append(content, "╬");

    append_repeat(content, "═", wEmail + 2);
    g_string_append(content, "╬");

    append_repeat(content, "═", wAddress + 2);

    g_string_append(content, "╣\n");

    // ===== Dữ liệu =====

    int stt = 1;

    for(int i = 0; i < slsv; i++) {

        if(strlen(filter) > 0 &&
           strcmp(ArrSinhVien[i].Class, filter) != 0)
            continue;

        char date_buf[20];

        sprintf(
            date_buf,
            "%02d/%02d/%04d",
            ArrSinhVien[i].NgaySinh.day,
            ArrSinhVien[i].NgaySinh.month,
            ArrSinhVien[i].NgaySinh.year
        );

        g_string_append_printf(
            content,
            "║ %-*d ║ %-*lld ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║ %-*s ║\n",

            wSTT, stt++,
            wMSSV, ArrSinhVien[i].MaSV,
            wName, ArrSinhVien[i].Name,
            wClass, ArrSinhVien[i].Class,
            wGender, ArrSinhVien[i].Gender,
            wDate, date_buf,
            wEmail, ArrSinhVien[i].Email,
            wAddress, ArrSinhVien[i].Address
        );
    }

    // ===== Đáy bảng =====

    g_string_append(content, "╚");

    append_repeat(content, "═", wSTT + 2);
    g_string_append(content, "╩");

    append_repeat(content, "═", wMSSV + 2);
    g_string_append(content, "╩");

    append_repeat(content, "═", wName + 2);
    g_string_append(content, "╩");

    append_repeat(content, "═", wClass + 2);
    g_string_append(content, "╩");

    append_repeat(content, "═", wGender + 2);
    g_string_append(content, "╩");

    append_repeat(content, "═", wDate + 2);
    g_string_append(content, "╩");

    append_repeat(content, "═", wEmail + 2);
    g_string_append(content, "╩");

    append_repeat(content, "═", wAddress + 2);

    g_string_append(content, "╝\n");

    return content;
}

/// ======= BUTTON QUAY LẠI =======
void on_back_clicked(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    // Xóa thông báo khi quay lại
    gtk_label_set_text(GTK_LABEL(app->msg_label), "");
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "menu");
}

void on_go_file_select(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;

    load_last_filename();
    LuuVaoFile(ArrSinhVien, slsv);
    g_autofree char *current_text = g_strdup_printf("<span size='x-large'>File hiện tại: <b>%s</b></span>", CurrentFileName);
    gtk_label_set_markup(GTK_LABEL(app->oldFileLabel), current_text);

    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "file_select");
}

/// ======= LẤY VÀ NHẬP THÔNG TIN TỪ FILE =======

void on_use_old_file(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    if (strlen(CurrentFileName) == 0) {
        const char *msg = "⚠️ Chưa có file cũ. Vui lòng tạo file mới!";
        if (app->fileMsgLabel) {
            char *m = g_strdup_printf("<span foreground='red' weight='bold'>%s</span>", msg);
            gtk_label_set_markup(GTK_LABEL(app->fileMsgLabel), m);
            g_free(m);
        } else show_message(app, msg, "red");
        return;
    }
    FILE *f_old = fopen(CurrentFileName, "r");
    if (f_old == NULL) {
        const char *msg = "⚠️ File cũ không tồn tại. Vui lòng tạo file mới!";
        if (app->fileMsgLabel) {
            char *m = g_strdup_printf("<span foreground='red' weight='bold'>%s</span>", msg);
            gtk_label_set_markup(GTK_LABEL(app->fileMsgLabel), m);
            g_free(m);
        } else show_message(app, msg, "red");
        return;
    }
    fclose(f_old);
    if (NhapTuFile(ArrSinhVien, &slsv, CurrentFileName) == 0) {
        const char *msg = "⚠️ Không thể mở file cũ. Vui lòng tạo mới!";
        if (app->fileMsgLabel) {
            char *m = g_strdup_printf("<span foreground='red' weight='bold'>%s</span>", msg);
            gtk_label_set_markup(GTK_LABEL(app->fileMsgLabel), m);
            g_free(m);
        } else show_message(app, msg, "red");
        return;
    }
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "menu");
}

void on_use_new_file(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    const char *newFile = gtk_editable_get_text(GTK_EDITABLE(app->fileEntry));
    if (strlen(newFile) == 0) {
        const char *msg = "⚠️ Vui lòng nhập tên file!";
        if (app->fileMsgLabel) {
            char *m = g_strdup_printf("<span foreground='red' weight='bold'>%s</span>", msg);
            gtk_label_set_markup(GTK_LABEL(app->fileMsgLabel), m);
            g_free(m);
        } else show_message(app, msg, "red");
        return;
    }
    FILE *f_new = fopen(newFile, "r");
    if (f_new == NULL) {
        const char *msg = "⚠️ File không tồn tại!";
        if (app->fileMsgLabel) {
            char *m = g_strdup_printf("<span foreground='red' weight='bold'>%s</span>", msg);
            gtk_label_set_markup(GTK_LABEL(app->fileMsgLabel), m);
            g_free(m);
        } else show_message(app, msg, "red");
        return;
    }
    fclose(f_new);
    if (NhapTuFile(ArrSinhVien, &slsv, newFile) == 0){
        const char *msg = "⚠️ Không thể mở file đã nhập!";
        if (app->fileMsgLabel) {
            char *m = g_strdup_printf("<span foreground='red' weight='bold'>%s</span>", msg);
            gtk_label_set_markup(GTK_LABEL(app->fileMsgLabel), m);
            g_free(m);
        } else show_message(app, msg, "red");
        return;
    }
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "menu");
}

GtkWidget* create_file_selection_ui(SinhvienWidgets *w) {

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    set_margin_all(vbox, 40);

    /// Author info
    GtkWidget *author = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(author),
        "<span size='xx-large' foreground='#ffffff' weight='bold' style='italic'>"
        "Thực hiện bởi: Trần Khánh Duy &amp; Lã Trung Thực"
        "</span>");
    gtk_label_set_xalign(GTK_LABEL(author), 0.5);
    gtk_widget_set_margin_bottom(author, 20);

    gtk_box_append(GTK_BOX(vbox), author);

    // Header
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='xx-large' weight='bold'>CHỌN FILE DỮ LIỆU</span>");
    gtk_widget_set_margin_bottom(header, 30);
    gtk_box_append(GTK_BOX(vbox), header);

    // Current file info
    GtkWidget *current_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_bottom(current_box, 20);
    w->oldFileLabel = gtk_label_new(NULL);
    char *current_text = g_strdup_printf("<span size='x-large'>File hiện tại: <b>%s</b></span>", CurrentFileName);
    gtk_label_set_markup(GTK_LABEL(w->oldFileLabel), current_text);
    g_free(current_text);
    gtk_box_append(GTK_BOX(current_box), w->oldFileLabel);
    gtk_box_append(GTK_BOX(vbox), current_box);

    // New file input
    GtkWidget *new_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(new_box, GTK_ALIGN_CENTER);
    GtkWidget *new_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(new_label), "<span size='x-large'>Hoặc nhập tên file mới:</span>");
    gtk_box_append(GTK_BOX(new_box), new_label);

    w->fileEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(w->fileEntry), "VD: sinhvien.txt");
    gtk_widget_set_size_request(w->fileEntry, 300, 40);
    gtk_box_append(GTK_BOX(new_box), w->fileEntry);

    GtkWidget *btn_new = gtk_button_new_with_label("SỬ DỤNG FILE MỚI");
    gtk_widget_add_css_class(btn_new, "label-btn");
    gtk_widget_add_css_class(btn_new, "border-label");
    gtk_widget_set_size_request(btn_new, -1, 50);
    g_signal_connect(btn_new, "clicked", G_CALLBACK(on_use_new_file), w);
    gtk_box_append(GTK_BOX(new_box), btn_new);

    gtk_box_append(GTK_BOX(vbox), new_box);

    // Separator
    GtkWidget *sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_margin_top(sep, 20);
    gtk_widget_set_margin_bottom(sep, 20);
    gtk_box_append(GTK_BOX(vbox), sep);

    // Use old file button
    GtkWidget *btn_old = gtk_button_new_with_label("TIẾP TỤC VỚI FILE HIỆN TẠI");
    gtk_widget_add_css_class(btn_old, "label-btn");
    gtk_widget_add_css_class(btn_old, "border-label");
    gtk_widget_set_size_request(btn_old, 400, 50);
    gtk_widget_set_halign(btn_old, GTK_ALIGN_CENTER);
    g_signal_connect(btn_old, "clicked", G_CALLBACK(on_use_old_file), w);
    gtk_box_append(GTK_BOX(vbox), btn_old);

    // Message label for file selection errors / info
    w->fileMsgLabel = gtk_label_new("");
    gtk_widget_set_margin_top(w->fileMsgLabel, 12);
    gtk_box_append(GTK_BOX(vbox), w->fileMsgLabel);

    return vbox;
}

/// ======= CHỨC NĂNG TÌM KIẾM SINH VIÊN =======
void ThemSinhVienCore(struct SinhVien Arrsinhvien[], int *count, struct SinhVien sv) {
    Arrsinhvien[*count] = sv;
    (*count)++;
}

/// ======= THÊM SINH VIÊN =======
void on_go_add(GtkWidget *w, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "add");
}

void on_add_clicked(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    struct SinhVien sv;

    const char *name = gtk_editable_get_text(GTK_EDITABLE(app->entryName));
    const char *date = gtk_editable_get_text(GTK_EDITABLE(app->entryDate));
    
    // Lấy giá trị từ dropdown
    int selected_pos = gtk_drop_down_get_selected(GTK_DROP_DOWN(app->entryGender));
    const char *gender = "";
    if (selected_pos == 1) {
        gender = "Nam";
    } else if (selected_pos == 2) {
        gender = "Nữ";
    } else if (selected_pos == 0){
        gender = "";
    }
    
    const char *address = gtk_editable_get_text(GTK_EDITABLE(app->entryAddress));
    const char *rawClassName = gtk_editable_get_text(GTK_EDITABLE(app->entryClass));
    char className[100] = "";
    trim_whitespace(className, rawClassName);

    // Kiểm tra dữ liệu
    int d, m, y;
    if (strlen(name) == 0) {
        show_message(app, "⚠️ Không được để trống Tên!", "red");
        return;
    }
    if (sscanf(date, "%d/%d/%d", &d, &m, &y) != 3) {
        show_message(app, "⚠️ Sai định dạng ngày (DD/MM/YYYY)!", "red");
        return;
    } else{
        if (checkDate(d, m, y) == -1) {
            show_message(app, "⚠️ Sinh viên có độ tuổi không phù hợp theo quy định!", "red");
            return;
        }
        else if (checkDate(d, m, y) == -2){
            show_message(app, "⚠️ Tháng sinh không phù hợp!", "red");
            return;
        }
        else if (checkDate(d, m, y) == -3){
            show_message(app, "⚠️ Ngày sinh không phù hợp hoặc không tồn tại!", "red");
            return;
        }
    }
    if (strlen(gender) == 0) {
        show_message(app, "⚠️ Sai định dạng giới tính!", "red");
        return;
    }
    
    char Khoa[100], TenLop[100];
    TachLop((char *)className, Khoa, TenLop);
    if (strlen(className) == 0 || checkClass(Khoa) == 0 || layMaKhoaTuLop(TenLop) == 0) {
        show_message(app, "⚠️ Sai định dạng tên Lớp hoặc không tồn lại Lớp!", "red");
        return;
    }

    if (!class_name_has_section(className)) {
        show_message(app, "⚠️ Sai định dạng lớp. Lớp phải kết thúc bằng chữ số 1-9.", "red");
        return;
    }

    int classSize = count_students_in_class(ArrSinhVien, slsv, className);
    if (classSize >= 100) {
        char msg[512];
        build_class_limit_message(ArrSinhVien, slsv, className, msg, sizeof(msg));
        show_message(app, msg, "yellow");
        return;
    }

    // Gán dữ liệu vào struct
    strcpy(sv.Name, name);
    vietHoaTatCa(sv.Name);
    strcpy(sv.Gender, gender);
    vietHoaChuDau(sv.Gender);
    strcpy(sv.Address, address);
    vietHoaChuDau(sv.Address);
    strcpy(sv.Class, className);
    vietHoaTatCa(sv.Class);
    strcpy(sv.Email, "Chưa có");
    sv.NgaySinh.day = d;
    sv.NgaySinh.month = m;
    sv.NgaySinh.year = y;
    sv.MaSV = 0;
    sv.MaKhoa = layMaKhoaTuLop(sv.Class);

    // Lưu vào mảng
    ThemSinhVienCore(ArrSinhVien, &slsv, sv);
    DaSapXep = 0;
    LuuVaoFile(ArrSinhVien, slsv);
    
    show_message(app, "✅ Thêm sinh viên thành công!", "green");

    // Clear input
    gtk_editable_set_text(GTK_EDITABLE(app->entryName), "");
    gtk_editable_set_text(GTK_EDITABLE(app->entryDate), "");
    gtk_drop_down_set_selected(GTK_DROP_DOWN(app->entryGender), -1);
    gtk_editable_set_text(GTK_EDITABLE(app->entryAddress), "");
    gtk_editable_set_text(GTK_EDITABLE(app->entryClass), "");
}

/// ======= GIAO DIỆN THÊM SINH VIÊN =======
GtkWidget* create_input_row(const char *label_text, GtkWidget **entry_out, const char *placeholder) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_bottom(vbox, 12);

    GtkWidget *lbl = gtk_label_new(NULL);
    char *markup = g_strdup_printf("<span size='small' weight='bold' color='#555555'>%s</span>", label_text);
    gtk_label_set_markup(GTK_LABEL(lbl), markup);
    gtk_label_set_xalign(GTK_LABEL(lbl), 0);
    g_free(markup);

    *entry_out = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(*entry_out), placeholder);
    gtk_widget_set_size_request(*entry_out, 350, 40);

    gtk_box_append(GTK_BOX(vbox), lbl);
    gtk_box_append(GTK_BOX(vbox), *entry_out);
    return vbox;
}

/// ======= DROPDOWN GIỚI TÍNH =======
GtkWidget* create_gender_dropdown_row(const char *label_text, GtkWidget **dropdown_out) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_bottom(vbox, 12);

    GtkWidget *lbl = gtk_label_new(NULL);
    char* markup = g_strdup_printf("<span size='medium' font_desc='Sans 14' weight='bold' color='#333333'>%s</span> <span font_desc='Sans 14' foreground='red'>*</span>", label_text);    
    gtk_label_set_markup(GTK_LABEL(lbl), markup);
    gtk_label_set_xalign(GTK_LABEL(lbl), 0);
    g_free(markup);

    *dropdown_out = gtk_drop_down_new_from_strings((const char * const[]){"", "Nam", "Nữ", NULL});
    gtk_widget_set_size_request(*dropdown_out, 350, 40);

    gtk_box_append(GTK_BOX(vbox), lbl);
    gtk_box_append(GTK_BOX(vbox), *dropdown_out);
    return vbox;
}
// Hàm này hỗ trợ truyền vào tiêu đề, và tự động thêm dấu * màu đỏ nếu là trường bắt buộc
GtkWidget* create_input_row_ui_add(const char *label_text, GtkWidget **out_entry, const char *placeholder, gboolean is_required) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_margin_bottom(box, 15); // Tạo khoảng cách giữa các hàng

    // Tạo Label với Pango Markup
    GtkWidget *label = gtk_label_new(NULL);
    char *markup;
    
    if (is_required) {
        markup = g_strdup_printf("<span size = 'medium' font_desc='Sans 14'><b>%s</b> <span foreground='red'>*</span></span>",label_text);
    } else {
        markup = g_strdup_printf("<span size = 'medium' font_desc='Sans 14'><b>%s</b></span>",label_text);
    }
    
    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);
    
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(box), label);
    
    // Tạo Entry
    *out_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(*out_entry), placeholder);
    gtk_widget_set_size_request(*out_entry, 300, 40); // Đặt kích thước cho đồng bộ
    gtk_box_append(GTK_BOX(box), *out_entry);
    
    return box;
}

GtkWidget* create_add_sv_ui(SinhvienWidgets *widgets) {
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    set_margin_all(main_vbox, 40);

    // Icon + Header
    GtkWidget *icon_header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(icon_header_box, GTK_ALIGN_CENTER);
    GtkWidget *icon = gtk_image_new_from_file(ADD_ICON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 48);
    gtk_box_append(GTK_BOX(icon_header_box), icon);
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='xx-large' weight='bold'>NHẬP THÔNG TIN</span>");
    gtk_box_append(GTK_BOX(icon_header_box), header);
    gtk_widget_set_margin_bottom(icon_header_box, 30);
    gtk_box_append(GTK_BOX(main_vbox), icon_header_box);

    GtkWidget *instr = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(instr), "<span size='x-large' weight='bold' color='#ff0000'> (*) KHÔNG ĐƯỢC ĐỂ TRỐNG CÁC THÔNG TIN BẮT BUỘC\n</span>");
    gtk_label_set_xalign(GTK_LABEL(instr), 0.5);
    gtk_box_append(GTK_BOX(main_vbox), instr);

    // Form container (Căn giữa)
    GtkWidget *form_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_halign(form_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(form_box, 20);
    gtk_box_append(GTK_BOX(main_vbox), form_box);

    // Các trường nhập liệu (với cờ is_required)
    // Những trường nào cần đánh dấu sao, bạn để là TRUE
    gtk_box_append(GTK_BOX(form_box), create_input_row_ui_add("HỌ VÀ TÊN", &widgets->entryName, "VD: Nguyễn Văn A", TRUE));
    gtk_box_append(GTK_BOX(form_box), create_input_row_ui_add("NGÀY SINH", &widgets->entryDate, "DD/MM/YYYY", TRUE));

    // Giới tính (Nếu bạn muốn dùng dropdown, hãy đảm bảo hàm create_gender_dropdown_row cũng hỗ trợ logic tương tự)
    gtk_box_append(GTK_BOX(form_box), create_gender_dropdown_row("GIỚI TÍNH", &widgets->entryGender));

    gtk_box_append(GTK_BOX(form_box), create_input_row_ui_add("ĐỊA CHỈ", &widgets->entryAddress, "Quê quán...", FALSE));
    gtk_box_append(GTK_BOX(form_box), create_input_row_ui_add("LỚP", &widgets->entryClass, "VD: 22T_DT1", TRUE));

    // Label thông báo
    widgets->msg_label = gtk_label_new("");
    gtk_widget_set_margin_bottom(widgets->msg_label, 15);
    gtk_box_append(GTK_BOX(form_box), widgets->msg_label);

    // Nút Add
    GtkWidget *btn_add = gtk_button_new_with_label("THÊM VÀO HỆ THỐNG");
    gtk_widget_add_css_class(btn_add, "label-btn");
    gtk_widget_add_css_class(btn_add, "border-label");
    gtk_widget_set_size_request(btn_add, -1, 50);
    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_add_clicked), widgets);
    gtk_box_append(GTK_BOX(form_box), btn_add);

    // Nút Back
    GtkWidget *btn_back = gtk_button_new_with_label("◀ Quay lại Menu");
    gtk_widget_add_css_class(btn_back, "label-btn");
    gtk_widget_add_css_class(btn_back, "border-label");
    gtk_button_set_has_frame(GTK_BUTTON(btn_back), FALSE);
    gtk_widget_set_margin_top(btn_back, 10);
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_clicked), widgets);
    gtk_widget_add_css_class(btn_back, "back-button");
    gtk_box_append(GTK_BOX(form_box), btn_back);

    return main_vbox;
}

/// ======= GIAO DIỆN DANH SÁCH SINH VIÊN =======
/// Preview trước khi xuất file
GtkWidget* create_preview_ui(SinhvienWidgets *app) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    set_margin_all(vbox, 20);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>XEM TRƯỚC DANH SÁCH SINH VIÊN</span>");
    gtk_widget_set_margin_bottom(title, 20);
    gtk_box_append(GTK_BOX(vbox), title);

    GtkWidget *scroll = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_hexpand(scroll, TRUE); // Cho phép vùng cuộn giãn hết chiều ngang

    app->previewText = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->previewText), FALSE); 
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(app->previewText), FALSE); 
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->previewText), GTK_WRAP_WORD_CHAR); 

    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(app->previewText), 10);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(app->previewText), 10);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(app->previewText), 10);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(app->previewText), 10);

    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), app->previewText);
    gtk_box_append(GTK_BOX(vbox), scroll);

    GtkWidget *btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(btn_box, 10);

    GtkWidget *btn_back = gtk_button_new_with_label("◀ Quay lại Menu");
    gtk_widget_add_css_class(btn_back, "label-btn");
    gtk_widget_add_css_class(btn_back, "border-label");
    gtk_widget_add_css_class(btn_back, "back-button");

    GtkWidget *btn_export = gtk_button_new_with_label("Xuất file");
    gtk_widget_add_css_class(btn_export, "label-btn");
    gtk_widget_add_css_class(btn_export, "border-label");

    gtk_box_append(GTK_BOX(btn_box), btn_back);
    gtk_box_append(GTK_BOX(btn_box), btn_export);

    gtk_box_append(GTK_BOX(vbox), btn_box);
    return vbox;
}
/// Giao diện danh sách sinh viên
void refresh_student_list(SinhvienWidgets *app, const char *className) {
    GtkWidget *child = gtk_widget_get_first_child(app->list_box);
    while (child) {
        gtk_flow_box_remove(GTK_FLOW_BOX(app->list_box), child);
        child = gtk_widget_get_first_child(app->list_box);
    }

    int found = 0;
    char class_filter[50] = "";
    if (className != NULL && strlen(className) > 0) {
        strcpy(class_filter, className);
        vietHoaTatCa(class_filter);
    }

    for (int i = 0; i < slsv; i++) {
        if (strlen(class_filter) > 0 && strcmp(ArrSinhVien[i].Class, class_filter) != 0) {
            continue;
        }

        GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
        set_margin_all(card, 12);
        gtk_widget_set_size_request(card, 360, -1);
        gtk_widget_set_halign(card, GTK_ALIGN_START);
        gtk_widget_add_css_class(card, "student-card");
        
        GtkWidget *row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
        GtkWidget *icon = gtk_image_new_from_icon_name("avatar-default-symbolic");
        gtk_image_set_pixel_size(GTK_IMAGE(icon), 40);

        GtkWidget *lbl = gtk_label_new(NULL);
        gtk_label_set_wrap(GTK_LABEL(lbl), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(lbl), 24);
        char *txt = g_strdup_printf("<b>%s</b>\n<span size='medium' color='#666666'>MSSV: %lld | Lớp: %s | Năm sinh: %d\nGiới tính: %s | Địa chỉ: %s\nEmail: %s</span>", 
                                    ArrSinhVien[i].Name, ArrSinhVien[i].MaSV, ArrSinhVien[i].Class, ArrSinhVien[i].NgaySinh.year,
                                    ArrSinhVien[i].Gender, ArrSinhVien[i].Address, ArrSinhVien[i].Email);
        gtk_label_set_markup(GTK_LABEL(lbl), txt);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0);
        g_free(txt);

        gtk_box_append(GTK_BOX(row), icon);
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_box_append(GTK_BOX(card), row);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->list_box), card, -1);

        GtkWidget *flow_child = gtk_widget_get_parent(card);
        if (flow_child) {
            gtk_widget_set_halign(flow_child, GTK_ALIGN_START);
            gtk_widget_set_valign(flow_child, GTK_ALIGN_START);
        }

        found = 1;
    }

    if (strlen(class_filter) > 0 && !found) {
        GtkWidget *row = create_student_card_box(10);
        GtkWidget *lbl = gtk_label_new(NULL);
        char *txt = g_strdup_printf("<span size='large' weight='bold' color='#ff0000'>Không tìm thấy sinh viên nào trong lớp <b>%s</b>!</span>", class_filter);
        gtk_label_set_markup(GTK_LABEL(lbl), txt);  
        gtk_label_set_xalign(GTK_LABEL(lbl), 0);
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->list_box), row, -1);
        g_free(txt);
    }
    if (strlen(class_filter) == 0 && !found) {
        GtkWidget *row = create_student_card_box(10);
        GtkWidget *lbl = gtk_label_new(NULL);
        char *txt = g_strdup_printf("<span size='large' weight='bold' color='#ff0000'>Không có sinh viên nào trong hệ thống!</span>");
        gtk_label_set_markup(GTK_LABEL(lbl), txt);  
        gtk_label_set_xalign(GTK_LABEL(lbl), 0);
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->list_box), row, -1);
        g_free(txt);
    }
}

void on_go_list(GtkWidget *w, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    gtk_editable_set_text(GTK_EDITABLE(app->listFilterEntry), "");
    refresh_student_list(app, NULL);
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "list");
}

void on_filter_list_clicked(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    const char *className = gtk_editable_get_text(GTK_EDITABLE(app->listFilterEntry));
    if (strlen(className) == 0) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("⚠️ Lỗi");
        gtk_alert_dialog_set_detail(dialog, "⚠️ Vui lòng nhập tên lớp để lọc!");
        gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
        return;
    }
    refresh_student_list(app, className);
}

void on_clear_list_filter_clicked(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    gtk_editable_set_text(GTK_EDITABLE(app->listFilterEntry), "");
    refresh_student_list(app, NULL);
}

// void on_confirm_export(GtkWidget *widget, gpointer data) {
//     export_to_data();
// }
// void on_export_list_clicked(GtkWidget *widget, gpointer data) {
    
//     SinhvienWidgets *app = data;

//     char buffer[50000];
//     buffer[0] = '\0';

//     for (int i = 0; i < slsv; i++) {
//         char temp[1000];
//         sprintf(temp, "MSSV: %lld\n""Tên: %s\n""Lớp: %s\n\n", ArrSinhVien[i].MaSV, ArrSinhVien[i].Name, ArrSinhVien[i].Class);

//         strcat(buffer, temp);
//     }

//     GtkTextBuffer *tb = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->previewText));
//     gtk_text_buffer_set_text(tb, buffer, -1);

//     gtk_stack_set_visible_chile_name(GTK_STACK(app->stack), "preview");
// }

static void on_export_list_clicked(GtkWidget *widget, gpointer data)
{
    ExportPreviewData *exp = data;

    const char *filename = gtk_editable_get_text(GTK_EDITABLE(exp->entry_file));

    if (strlen(filename) == 0) {
        GtkAlertDialog *alert = gtk_alert_dialog_new("Thiếu tên file");
        gtk_alert_dialog_set_detail(alert, "Vui lòng nhập tên file.");
        gtk_alert_dialog_show(alert, GTK_WINDOW(exp->window));
        return;
    }

    if (!is_valid_export_filename(filename)) {
        GtkAlertDialog *alert = gtk_alert_dialog_new("Tên file không hợp lệ");
        gtk_alert_dialog_set_detail(alert, "Tên file phải kết thúc bằng .txt");
        gtk_alert_dialog_show(alert, GTK_WINDOW(exp->window));
        return;
    }

    const char *filter = gtk_editable_get_text(GTK_EDITABLE(exp->app->listFilterEntry));

    if (!export_student_list_to_file(filename,filter,ArrSinhVien,slsv)) {
        GtkAlertDialog *alert = gtk_alert_dialog_new("Xuất file thất bại");
        gtk_alert_dialog_set_detail(alert, "Không thể ghi file hoặc không có dữ liệu phù hợp.");
        gtk_alert_dialog_show(alert, GTK_WINDOW(exp->window));
        return;
    }

    char msg[256];

    snprintf(msg, sizeof(msg), "Đã xuất danh sách vào file:\n%s", filename);

    GtkAlertDialog *alert = gtk_alert_dialog_new("Xuất file thành công");
    gtk_alert_dialog_set_detail(alert, msg);
    gtk_alert_dialog_show(alert, GTK_WINDOW(exp->window));
}
void on_show_export_preview(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets*)data;

    GtkWidget *window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "Xem trước xuất file");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);

    GtkRoot *root = gtk_widget_get_root(widget);
    if (GTK_IS_WINDOW(root)) {
        gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(root));
        gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    }

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    set_margin_all(main_box, 20);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>XEM TRƯỚC DANH SÁCH SINH VIÊN</span>");
    gtk_box_append(GTK_BOX(main_box), title);

    char buf[100];
    sprintf(buf, "Tổng số sinh viên trong file %s là: %d", CurrentFileName, slsv);

    GtkWidget *info = gtk_label_new(buf);
    gtk_box_append(GTK_BOX(main_box), info);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_box_append(GTK_BOX(main_box), scrolled);

    GtkWidget *textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(textview), FALSE);
    gtk_widget_add_css_class(textview, "preview-text");
    
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(textview), TRUE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), textview);
    
    const char *classFilter = gtk_editable_get_text(GTK_EDITABLE(app->listFilterEntry));
    char filter[64] = "";
    if (classFilter != NULL && strlen(classFilter) > 0) {
        strncpy(filter, classFilter, sizeof(filter) - 1);
        filter[sizeof(filter) - 1] = '\0';
        vietHoaTatCa(filter);
    }
    
    int count = 0;
    for(int i = 0; i < slsv; i++) {
        if (strlen(filter) > 0 && strcmp(ArrSinhVien[i].Class, filter) != 0) continue;
        count++;
    }
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

    GString *content = g_string_new("");

    if (strlen(filter) == 0) g_string_append_printf(content, "Tổng số sinh viên trong file hiển thị: %d\n\n", count);
    else g_string_append_printf(content, "Tổng số sinh viên trong lớp %s hiển thị: %d\n\n", filter, count);
    int stt = 1;

    for (int i = 0; i < slsv; i++) {

        if (strlen(filter) > 0 &&
            strcmp(ArrSinhVien[i].Class, filter) != 0)
            continue;

        g_string_append_printf(
            content,

            "══════════════════════════════════════════════════════\n"
            "[%03d] \n"
            "MSSV      : %lld\n"
            "Họ tên    : %s\n"
            "Lớp       : %s\n"
            "Giới tính : %s\n"
            "Ngày sinh : %02d/%02d/%04d\n"
            "Email     : %s\n"
            "Địa chỉ   : %s\n\n",

            stt++,

            ArrSinhVien[i].MaSV,
            ArrSinhVien[i].Name,
            ArrSinhVien[i].Class,
            ArrSinhVien[i].Gender,

            ArrSinhVien[i].NgaySinh.day,
            ArrSinhVien[i].NgaySinh.month,
            ArrSinhVien[i].NgaySinh.year,

            ArrSinhVien[i].Email,
            ArrSinhVien[i].Address
        );
    }

    gtk_text_buffer_set_text(buffer, content->str, -1);
    g_string_free(content, TRUE); 
    
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    gtk_widget_set_valign(button_box, GTK_ALIGN_CENTER);

    GtkWidget *lbl_file = gtk_label_new("Tên file:");
    GtkWidget *entry_file = gtk_entry_new();
    gtk_widget_set_size_request(entry_file, 250, 35);
    
    if (strlen(filter) == 0) gtk_editable_set_text(GTK_EDITABLE(entry_file),CurrentFileName); 
    else {
        char default_filename[100];
        sprintf(default_filename, "danhsach_%s.txt", filter);
        gtk_editable_set_text(GTK_EDITABLE(entry_file), default_filename);
    }   

    GtkWidget *btn_close = gtk_button_new_with_label("Đóng");
    gtk_widget_set_size_request(btn_close, 80, 35);
    g_signal_connect_swapped(btn_close, "clicked", G_CALLBACK(gtk_window_destroy), window);

    gtk_box_append(GTK_BOX(button_box), lbl_file);
    gtk_box_append(GTK_BOX(button_box), entry_file);
    gtk_box_append(GTK_BOX(button_box), btn_close);

    ExportPreviewData *exp = g_malloc(sizeof(ExportPreviewData));
    exp->app = app;
    exp->entry_file = entry_file;
    exp->window = window;
    
    g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_free), exp);

    GtkWidget *btn_export = gtk_button_new_with_label("Xuất file");
    gtk_widget_set_size_request(btn_export, 90, 35);
    gtk_box_append(GTK_BOX(button_box), btn_export);
    
    g_signal_connect(btn_export, "clicked", G_CALLBACK(on_export_list_clicked), exp);

    gtk_box_append(GTK_BOX(main_box), button_box);
    
    gtk_window_present(GTK_WINDOW(window));
}

GtkWidget* create_list_sv_ui(SinhvienWidgets *widgets) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    set_margin_all(vbox, 20);

    // Icon + Title
    GtkWidget *icon_title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(icon_title_box, GTK_ALIGN_CENTER);
    GtkWidget *icon = gtk_image_new_from_file(LIST_ICON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 48);
    gtk_box_append(GTK_BOX(icon_title_box), icon);
    
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), "<span size='xx-large' weight='bold'>DANH SÁCH SINH VIÊN</span>");
    gtk_box_append(GTK_BOX(icon_title_box), title);
    gtk_widget_set_margin_bottom(icon_title_box, 20);
    gtk_box_append(GTK_BOX(vbox), icon_title_box);

    GtkWidget *filter_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(filter_box, 10);
    gtk_widget_set_halign(filter_box, GTK_ALIGN_FILL);

    //======================= left box
    GtkWidget *left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    
    // back button
    GtkWidget *btn_back = gtk_button_new_with_label("◀ Quay lại Menu");
    gtk_widget_add_css_class(btn_back, "label-btn");
    gtk_widget_add_css_class(btn_back, "border-label");
    gtk_widget_set_size_request(btn_back, 250, 50);
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_clicked), widgets);
    gtk_widget_add_css_class(btn_back, "back-button");
    gtk_box_append(GTK_BOX(left_box), btn_back);

    // space
    GtkWidget *space1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_hexpand(space1, TRUE);
    
    GtkWidget *filter_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(filter_label), "<span size = 'large' weight='bold'>LỌC LỚP: </span>");
    gtk_widget_set_size_request(filter_label, 80, -1);
    widgets->listFilterEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(widgets->listFilterEntry), "VD: 22T_DT1");
    gtk_widget_set_size_request(widgets->listFilterEntry, 200, 35);
    g_signal_connect(widgets->listFilterEntry, "activate", G_CALLBACK(on_filter_list_clicked), widgets);

    GtkWidget *btn_clear_filter = gtk_button_new_with_label("Xóa bộ lọc");
    gtk_widget_add_css_class(btn_clear_filter, "label-btn");
    gtk_widget_add_css_class(btn_clear_filter, "border-label");
    gtk_widget_set_size_request(btn_clear_filter, 120, 35);
    g_signal_connect(btn_clear_filter, "clicked", G_CALLBACK(on_clear_list_filter_clicked), widgets);

    GtkWidget *btn_export = gtk_button_new_with_label("Xuất file");
    gtk_widget_add_css_class(btn_export, "label-btn");
    gtk_widget_add_css_class(btn_export, "border-label");
    gtk_widget_set_size_request(btn_export, 90, 35);
    g_signal_connect(btn_export, "clicked", G_CALLBACK(on_show_export_preview), widgets);

    gtk_box_append(GTK_BOX(filter_box), left_box);
    gtk_box_append(GTK_BOX(filter_box), space1);
    gtk_box_append(GTK_BOX(filter_box), filter_label);
    gtk_box_append(GTK_BOX(filter_box), widgets->listFilterEntry);
    gtk_box_append(GTK_BOX(filter_box), btn_clear_filter);
    gtk_box_append(GTK_BOX(filter_box), btn_export);
    gtk_box_append(GTK_BOX(vbox), filter_box);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_size_request(scrolled, -1, 450);
    
    widgets->list_box = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(widgets->list_box), 5);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(widgets->list_box), 1);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(widgets->list_box), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(widgets->list_box), 12);
    gtk_widget_set_halign(widgets->list_box, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(widgets->list_box, TRUE);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), widgets->list_box);
    gtk_box_append(GTK_BOX(vbox), scrolled);


    return vbox;
}

/// ======= TÌM SINH VIEN =======
void on_go_search(GtkWidget *w, gpointer data){
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "search");
}

void refresh_search_results(SinhvienWidgets *app, int search_type) {
    // Xóa kết quả cũ
    GtkWidget *child = gtk_widget_get_first_child(app->searchListBox);
    while (child) {
        gtk_flow_box_remove(GTK_FLOW_BOX(app->searchListBox), child);
        child = gtk_widget_get_first_child(app->searchListBox);
    }

    const char *searchName = gtk_editable_get_text(GTK_EDITABLE(app->searchEntryName));
    const char *searchMSSV = gtk_editable_get_text(GTK_EDITABLE(app->searchEntryMSSV));
    
    int found = 0;

    if (search_type == 1 && strlen(searchName) > 0) {
        // Tìm theo tên
        char searchName_upper[50];
        strcpy(searchName_upper, searchName);
        vietHoaTatCa(searchName_upper);
        
        for (int i = 0; i < slsv; i++) {
            if (strcmp(layTen(ArrSinhVien[i].Name), searchName_upper) == 0) {
                GtkWidget *row = create_student_card_box(15);

                GtkWidget *icon = gtk_image_new_from_icon_name("avatar-default-symbolic");
                gtk_image_set_pixel_size(GTK_IMAGE(icon), 40);

                GtkWidget *lbl = gtk_label_new(NULL);
                char *txt = g_strdup_printf("<b>%s</b>\n<span size='medium' color='#666666'>MSSV: %lld | Lớp: %s\nGiới tính: %s | Địa chỉ: %s\nEmail: %s</span>", 
                                            ArrSinhVien[i].Name, ArrSinhVien[i].MaSV, ArrSinhVien[i].Class,
                                            ArrSinhVien[i].Gender, ArrSinhVien[i].Address, ArrSinhVien[i].Email);
                gtk_label_set_markup(GTK_LABEL(lbl), txt);
                gtk_label_set_xalign(GTK_LABEL(lbl), 0);
                g_free(txt);

                gtk_box_append(GTK_BOX(row), icon);
                gtk_box_append(GTK_BOX(row), lbl);
                gtk_flow_box_insert(GTK_FLOW_BOX(app->searchListBox), row, -1);
                found = 1;
            }
        }
    } else if (search_type == 2 && strlen(searchMSSV) > 0) {
        // Tìm theo MSSV
        long long maTim = atoll(searchMSSV);
        for (int i = 0; i < slsv; i++) {
            if (ArrSinhVien[i].MaSV == maTim) {
                GtkWidget *row = create_student_card_box(15);

                GtkWidget *icon = gtk_image_new_from_icon_name("avatar-default-symbolic");
                gtk_image_set_pixel_size(GTK_IMAGE(icon), 40);

                GtkWidget *lbl = gtk_label_new(NULL);
                char *txt = g_strdup_printf("<b>%s</b>\n<span size='medium' color='#666666'>MSSV: %lld | Lớp: %s\nGiới tính: %s | Địa chỉ: %s\nEmail: %s</span>", 
                                            ArrSinhVien[i].Name, ArrSinhVien[i].MaSV, ArrSinhVien[i].Class,
                                            ArrSinhVien[i].Gender, ArrSinhVien[i].Address, ArrSinhVien[i].Email);
                gtk_label_set_markup(GTK_LABEL(lbl), txt);
                gtk_label_set_xalign(GTK_LABEL(lbl), 0);
                g_free(txt);

                gtk_box_append(GTK_BOX(row), icon);
                gtk_box_append(GTK_BOX(row), lbl);
                gtk_flow_box_insert(GTK_FLOW_BOX(app->searchListBox), row, -1);
                found = 1;
                break;
            }
        }
    }

    if (!found) {
        GtkWidget *row = create_student_card_box(10);
        GtkWidget *lbl = gtk_label_new(NULL);
        char *smg = g_strdup_printf("<span size = 'large' weight = 'bold' color = 'red'> ❌ Không tìm thấy sinh viên nào! </span>");
        gtk_label_set_markup(GTK_LABEL(lbl), smg);
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->searchListBox), row, -1);
    }
}

void on_search_by_name(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    refresh_search_results(app, 1);
}

void on_search_by_mssv(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    refresh_search_results(app, 2);
}

GtkWidget* create_search_sv_ui(SinhvienWidgets *w){
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    set_margin_all(vbox, 20);

    // Icon + Header
    GtkWidget *icon_header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(icon_header_box, GTK_ALIGN_CENTER);
    GtkWidget *icon = gtk_image_new_from_file(SEARCH_ICON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 48);
    gtk_box_append(GTK_BOX(icon_header_box), icon);
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='xx-large' weight='bold'>TÌM KIẾM SINH VIÊN</span>");
    gtk_box_append(GTK_BOX(icon_header_box), header);
    gtk_widget_set_margin_bottom(icon_header_box, 20);
    gtk_box_append(GTK_BOX(vbox), icon_header_box);

    GtkWidget *instr = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(instr), "<span size='small' weight='bold' color='#15ff00'>Tìm kiếm sinh viên bằng Tên - MSSV\n</span>");
    gtk_label_set_xalign(GTK_LABEL(instr), 0.5);
    gtk_box_append(GTK_BOX(vbox), instr);

    // Search form container
    GtkWidget *form_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_halign(form_box, GTK_ALIGN_CENTER);

    // Search by name
    GtkWidget *name_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *name_label = gtk_label_new("Tìm theo tên:");
    gtk_widget_set_size_request(name_label, 120, -1);
    w->searchEntryName = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(w->searchEntryName), "Nhập tên sinh viên...");
    gtk_widget_set_size_request(w->searchEntryName, 220, 35);
    GtkWidget *btn_search_name = gtk_button_new_with_label("Tìm");
    gtk_widget_set_size_request(btn_search_name, 60, 35);
    g_signal_connect(btn_search_name, "clicked", G_CALLBACK(on_search_by_name), w);
    
    gtk_box_append(GTK_BOX(name_box), name_label);
    gtk_box_append(GTK_BOX(name_box), w->searchEntryName);
    gtk_box_append(GTK_BOX(name_box), btn_search_name);
    gtk_box_append(GTK_BOX(form_box), name_box);

    // Separator
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(form_box), separator);

    // Search by MSSV
    GtkWidget *mssv_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *mssv_label = gtk_label_new("Tìm theo MSSV:");
    gtk_widget_set_size_request(mssv_label, 120, -1);
    w->searchEntryMSSV = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(w->searchEntryMSSV), "Nhập MSSV...");
    gtk_widget_set_size_request(w->searchEntryMSSV, 220, 35);
    GtkWidget *btn_search_mssv = gtk_button_new_with_label("Tìm");
    gtk_widget_set_size_request(btn_search_mssv, 60, 35);
    g_signal_connect(btn_search_mssv, "clicked", G_CALLBACK(on_search_by_mssv), w);
    
    gtk_box_append(GTK_BOX(mssv_box), mssv_label);
    gtk_box_append(GTK_BOX(mssv_box), w->searchEntryMSSV);
    gtk_box_append(GTK_BOX(mssv_box), btn_search_mssv);
    gtk_box_append(GTK_BOX(form_box), mssv_box);

    gtk_box_append(GTK_BOX(vbox), form_box);

    // Search results
    GtkWidget *results_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(results_label), "<span size='small' weight='bold' color='white'>Kết quả tìm kiếm:</span>");
    gtk_label_set_xalign(GTK_LABEL(results_label), 0);
    gtk_widget_set_margin_top(results_label, 15);
    gtk_box_append(GTK_BOX(vbox), results_label);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    // FIX LỖI 1: Dùng size_request kết hợp vexpand để giữ nút ở dưới cuối cửa sổ
    gtk_widget_set_size_request(scrolled, -1, 300);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    w->searchListBox = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(w->searchListBox), 5);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(w->searchListBox), 1);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(w->searchListBox), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(w->searchListBox), 12);
    gtk_widget_set_halign(w->searchListBox, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(w->searchListBox, TRUE);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), w->searchListBox);
    gtk_box_append(GTK_BOX(vbox), scrolled);

    // Back button
    GtkWidget *btn_back = gtk_button_new_with_label("Quay lại Menu");
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_clicked), w);
    gtk_widget_set_halign(btn_back, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(btn_back, 10);
    gtk_widget_add_css_class(btn_back, "back-button");
    gtk_box_append(GTK_BOX(vbox), btn_back);

    return vbox;
}

/// ======= XÓA SINH VIÊN =======
void on_go_delete(GtkWidget *w, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    refresh_delete_list(app);
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "delete");
}

void refresh_delete_list(SinhvienWidgets *app) {
    GtkWidget *child = gtk_widget_get_first_child(app->deleteListBox);
    while (child) {
        gtk_flow_box_remove(GTK_FLOW_BOX(app->deleteListBox), child);
        child = gtk_widget_get_first_child(app->deleteListBox);
    }
    
    gtk_label_set_text(GTK_LABEL(app->deleteMsg), "");
    app->selectedDeleteIndex = -1;

    if (slsv == 0) {
        GtkWidget *row = create_student_card_box(10);
        GtkWidget *lbl = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl), "<span size='large' weight='bold' color='#ff0000'>❌ Không có sinh viên nào!</span>");
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->deleteListBox), row, -1);
        return;
    }

    for (int i = 0; i < slsv; i++) {
        GtkWidget *row = create_student_card_box(15);

        GtkWidget *icon = gtk_image_new_from_icon_name("avatar-default-symbolic");
        gtk_image_set_pixel_size(GTK_IMAGE(icon), 40);

        GtkWidget *lbl = gtk_label_new(NULL);
        char *txt = g_strdup_printf("<b>%s</b>\n<span size='medium' color='#666666'>MSSV: %lld | Lớp: %s | Năm sinh: %d\nGiới tính: %s | Địa chỉ: %s\nEmail: %s</span>", 
                                    ArrSinhVien[i].Name, ArrSinhVien[i].MaSV, ArrSinhVien[i].Class, ArrSinhVien[i].NgaySinh.year,
                                    ArrSinhVien[i].Gender, ArrSinhVien[i].Address, ArrSinhVien[i].Email);
        gtk_label_set_markup(GTK_LABEL(lbl), txt);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0);
        g_free(txt);

        gtk_box_append(GTK_BOX(row), icon);
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->deleteListBox), row, -1);
        GtkWidget *flow_child = gtk_widget_get_parent(row);
        if (flow_child) {
            g_object_set_data(G_OBJECT(flow_child), "student-index", GINT_TO_POINTER(i + 1));
        }
    }
}

static int get_selected_delete_index(SinhvienWidgets *app) {
    GList *selected_children = gtk_flow_box_get_selected_children(GTK_FLOW_BOX(app->deleteListBox));
    int index = -1;
    if (selected_children) {
        GtkFlowBoxChild *child = GTK_FLOW_BOX_CHILD(selected_children->data);
        gpointer stored_index = g_object_get_data(G_OBJECT(child), "student-index");
        if (stored_index) {
            index = GPOINTER_TO_INT(stored_index) - 1;
        }
        g_list_free(selected_children);
    }
    return index;
}

/* deleted: remove delete-by-name flow (replaced by search/edit flow) */

// void on_delete_by_mssv(GtkWidget *widget, gpointer data) {
//     SinhvienWidgets *app = (SinhvienWidgets *)data;
//     const char *mssv_text = gtk_editable_get_text(GTK_EDITABLE(app->deleteMSSVEntry));
//     if (strlen(mssv_text) == 0) {
//         char *markup = g_strdup_printf("<span color='red' weight='bold'>⚠️ Vui lòng nhập MSSV!</span>");
//         gtk_label_set_markup(GTK_LABEL(app->deleteMsg), markup);
//         g_free(markup);
//         return;
//     }

//     long long mssv = atoll(mssv_text);
//     DeleteData *dd = g_new(DeleteData, 1);
//     dd->app = app;
//     dd->mssv = mssv;

//     GtkAlertDialog *dialog = gtk_alert_dialog_new("❗️ Xác nhận xóa");
//     gtk_alert_dialog_set_detail(dialog, "‼️ Bạn có chắc chắn muốn xóa sinh viên có MSSV này?");
//     gtk_alert_dialog_set_buttons(dialog, (const char *[]){"Hủy", "Xóa", NULL});
//     gtk_alert_dialog_choose(dialog, GTK_WINDOW(gtk_widget_get_root(widget)), NULL, on_delete_by_mssv_confirm, dd);
// }

void on_delete_clicked(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    int selectedIndex = get_selected_delete_index(app);

    if (selectedIndex < 0) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("⚠️ Chưa chọn sinh viên");
        gtk_alert_dialog_set_detail(dialog, "⚠️ Vui lòng chọn một sinh viên từ danh sách để xóa!");
        gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
        return;
    }

    if (selectedIndex >= slsv) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("❗️ Lỗi");
        gtk_alert_dialog_set_detail(dialog, "‼️ Sinh viên này không còn tồn tại!");
        gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
        return;
    }

    DeleteData *dd = g_new(DeleteData, 1);
    dd->app = app;
    dd->selectedIndex = selectedIndex;

    GtkAlertDialog *dialog = gtk_alert_dialog_new("❗️ Xác nhận xóa");
    gtk_alert_dialog_set_detail(dialog, "‼️ Bạn có chắc chắn muốn xóa sinh viên này?");
    gtk_alert_dialog_set_buttons(dialog, (const char *[]){"Hủy", "Xóa", NULL});
    gtk_alert_dialog_choose(dialog, GTK_WINDOW(gtk_widget_get_root(widget)), NULL, on_delete_selected_confirm, dd);
}

/* deleted: old confirmation handler for delete-by-name */

// static void on_delete_by_mssv_confirm(GObject *source, GAsyncResult *result, gpointer data) {
//     DeleteData *dd = (DeleteData *)data;
//     int response = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), result, NULL);
//     if (response == 1) {
//         int found = 0;
//         for (int i = 0; i < slsv; i++) {
//             if (ArrSinhVien[i].MaSV == dd->mssv) {
//                 for (int j = i; j < slsv - 1; j++) {
//                     ArrSinhVien[j] = ArrSinhVien[j + 1];
//                 }
//                 slsv--;
//                 found = 1;
//                 break;
//             }
//         }
//         if (found) {
//             LuuVaoFile(ArrSinhVien, slsv);
//             DaSapXep = 0;
//             // Hiện dialog thông báo thành công
//             GtkAlertDialog *success_dialog = gtk_alert_dialog_new("🎉 Xóa thành công");
//             char *detail = g_strdup_printf("🎉 Đã xóa sinh viên có MSSV %lld.", dd->mssv);
//             gtk_alert_dialog_set_detail(success_dialog, detail);
//             gtk_alert_dialog_set_buttons(success_dialog, (const char *[]){
//                 "OK", NULL});
//             gtk_alert_dialog_show(success_dialog, GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(dd->app->deleteEntry))));
//             g_free(detail);
//         } else {
//             GtkAlertDialog *error_dialog = gtk_alert_dialog_new("⚠️ Lỗi");
//             gtk_alert_dialog_set_detail(error_dialog, "⚠️ Không tìm thấy MSSV này!");
//             gtk_alert_dialog_set_buttons(error_dialog, (const char *[]){
//                 "OK", NULL});
//             gtk_alert_dialog_show(error_dialog, GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(dd->app->deleteMSSVEntry))));
//         }
//         gtk_editable_set_text(GTK_EDITABLE(dd->app->deleteMSSVEntry), "");
//         refresh_delete_list(dd->app);
//     }
//     g_free(dd);
// }

static void on_delete_selected_confirm(GObject *source, GAsyncResult *result, gpointer data) {
    DeleteData *dd = (DeleteData *)data;
    int response = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), result, NULL);
    if (response == 1) {
        for (int i = dd->selectedIndex; i < slsv - 1; i++) {
            ArrSinhVien[i] = ArrSinhVien[i + 1];
        }
        slsv--;
        LuuVaoFile(ArrSinhVien, slsv);
        DaSapXep = 0;
        // Hiện dialog thông báo thành công
        GtkAlertDialog *success_dialog = gtk_alert_dialog_new("🎉 Xóa thành công");
        gtk_alert_dialog_set_detail(success_dialog, "🎉 Đã xóa sinh viên thành công!");
        gtk_alert_dialog_set_buttons(success_dialog, (const char *[]){
            "OK", NULL});
        gtk_alert_dialog_show(success_dialog, GTK_WINDOW(gtk_widget_get_root(GTK_WIDGET(dd->app->deleteListBox))));
        refresh_delete_list(dd->app);
    }
    g_free(dd);
}

static void on_edit_save(GtkWidget *widget, gpointer user_data) {
    EditDialogData *ed = (EditDialogData *)user_data;
    if (!ed) return;
    int idx = ed->index;
    const char *name = gtk_editable_get_text(GTK_EDITABLE(ed->entryName));
    const char *date = gtk_editable_get_text(GTK_EDITABLE(ed->entryDate));
    int sel = gtk_drop_down_get_selected(GTK_DROP_DOWN(ed->entryGender));
    const char *gender = "";
    if (sel == 1) gender = "Nam";
    else if (sel == 2) gender = "Nữ";
    const char *addr = gtk_editable_get_text(GTK_EDITABLE(ed->entryAddress));
    const char *class = gtk_editable_get_text(GTK_EDITABLE(ed->entryClass));
    const char *email = gtk_editable_get_text(GTK_EDITABLE(ed->entryEmail));

    if (idx < 0 || idx >= slsv) {
        GtkAlertDialog *err = gtk_alert_dialog_new("Loi");
        gtk_alert_dialog_set_detail(err, "Sinh vien nay khong con ton tai.");
        gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
        return;
    }

    if (strlen(name) >= sizeof(ArrSinhVien[idx].Name) ||
        strlen(addr) >= sizeof(ArrSinhVien[idx].Address) ||
        strlen(class) >= sizeof(ArrSinhVien[idx].Class) ||
        strlen(email) >= sizeof(ArrSinhVien[idx].Email)) {
        GtkAlertDialog *err = gtk_alert_dialog_new("Loi");
        gtk_alert_dialog_set_detail(err, "Thong tin nhap qua dai. Vui long kiem tra lai.");
        gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
        return;
    }

    char nameBuf[sizeof(ArrSinhVien[idx].Name)];
    char addrBuf[sizeof(ArrSinhVien[idx].Address)];
    char classBuf[sizeof(ArrSinhVien[idx].Class)];
    char emailBuf[sizeof(ArrSinhVien[idx].Email)];
    trim_whitespace(nameBuf, name);
    trim_whitespace(addrBuf, addr);
    trim_whitespace(classBuf, class);
    trim_whitespace(emailBuf, email);
    vietHoaTatCa(classBuf);

    if (strlen(nameBuf) == 0) {
        GtkAlertDialog *err = gtk_alert_dialog_new("Lỗi");
        gtk_alert_dialog_set_detail(err, "Tên không được để trống.");
        gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
        return;
    }

    if (sel != 1 && sel != 2) {
        GtkAlertDialog *err = gtk_alert_dialog_new("Loi");
        gtk_alert_dialog_set_detail(err, "Vui long chon gioi tinh.");
        gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
        return;
    }
    if (strlen(addrBuf) == 0) {
        GtkAlertDialog *err = gtk_alert_dialog_new("Loi");
        gtk_alert_dialog_set_detail(err, "Dia chi khong duoc de trong.");
        gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
        return;
    }
    if (strlen(classBuf) == 0) {
        GtkAlertDialog *err = gtk_alert_dialog_new("Loi lop");
        gtk_alert_dialog_set_detail(err, "Lop khong duoc de trong.");
        gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
        return;
    }
    if (strlen(emailBuf) == 0) {
        GtkAlertDialog *err = gtk_alert_dialog_new("Loi email");
        gtk_alert_dialog_set_detail(err, "Email khong duoc de trong.");
        gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
        return;
    }

    int d = 0, m = 0, y = 0;
    if (sscanf(date, "%d/%d/%d", &d, &m, &y) != 3 || checkDate(d, m, y) != 1) {
        GtkAlertDialog *err = gtk_alert_dialog_new("Lỗi ngày");
        gtk_alert_dialog_set_detail(err, "Định dạng ngày không hợp lệ (DD/MM/YYYY) hoặc ngày không hợp lệ.");
        gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
        return;
    }

    // Kiểm tra tính hợp lệ của tên lớp trước khi áp dụng thay đổi
    char KhoaNew[128] = {0}, TenLopNew[128] = {0};
    char KhoaOld[128] = {0}, TenLopOld[128] = {0};
    TachLop(classBuf, KhoaNew, TenLopNew);
    TachLop(ArrSinhVien[idx].Class, KhoaOld, TenLopOld);

    if (ArrSinhVien[idx].MaSV == 0) {
        // Nếu chưa có MSSV: cho phép đổi lớp nhưng phải hợp lệ (kiểm tra như khi thêm sinh viên)
        if (strlen(classBuf) == 0 || checkClass(KhoaNew) == 0 || layMaKhoaTuLop(TenLopNew) == 0) {
            GtkAlertDialog *err = gtk_alert_dialog_new("Lỗi lớp");
            gtk_alert_dialog_set_detail(err, "⚠️ Sai định dạng tên Lớp hoặc lớp không tồn tại!");
            gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
            gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
            return;
        }
        if (!class_name_has_section(classBuf)) {
            GtkAlertDialog *err = gtk_alert_dialog_new("Lỗi lớp");
            gtk_alert_dialog_set_detail(err, "⚠️ Sai định dạng lớp. Lớp phải kết thúc bằng chữ số 1-9.");
            gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
            gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
            return;
        }
        int classSize = count_students_in_class(ArrSinhVien, slsv, classBuf);
        if (strcmp(ArrSinhVien[idx].Class, classBuf) == 0 && classSize > 0) {
            classSize--;
        }
        if (classSize >= 100) {
            char msg[512];
            build_class_limit_message(ArrSinhVien, slsv, classBuf, msg, sizeof(msg));
            GtkAlertDialog *err = gtk_alert_dialog_new("Giới hạn lớp");
            gtk_alert_dialog_set_detail(err, msg);
            gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
            gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
            return;
        }
    } else {
        // Nếu đã có MSSV: không được đổi mã khóa (Khoa numeric) và mã khoa của lớp mới phải trùng với mã khoa cũ
        if (strcmp(KhoaNew, KhoaOld) != 0) {
            GtkAlertDialog *err = gtk_alert_dialog_new("Lỗi thay đổi khóa");
            gtk_alert_dialog_set_detail(err, "⚠️ Mã khóa không được đổi cho sinh viên đã có MSSV.");
            gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
            gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
            return;
        }
        int newMaKhoa = layMaKhoaTuLop(TenLopNew);
        if (newMaKhoa == 0 || newMaKhoa != ArrSinhVien[idx].MaKhoa) {
            GtkAlertDialog *err = gtk_alert_dialog_new("Lỗi thay đổi khoa");
            gtk_alert_dialog_set_detail(err, "⚠️ Lớp mới không thuộc mã khoa cũ của sinh viên.");
            gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
            gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
            return;
        }
        if (!class_name_has_section(classBuf)) {
            GtkAlertDialog *err = gtk_alert_dialog_new("Lỗi lớp");
            gtk_alert_dialog_set_detail(err, "⚠️ Sai định dạng lớp. Lớp phải kết thúc bằng chữ số 1-9.");
            gtk_alert_dialog_set_buttons(err, (const char *[]){"OK", NULL});
            gtk_alert_dialog_show(err, GTK_WINDOW(gtk_widget_get_root(ed->window)));
            return;
        }
    }

    // Apply changes
    strncpy(ArrSinhVien[idx].Name, nameBuf, sizeof(ArrSinhVien[idx].Name) - 1);
    ArrSinhVien[idx].Name[sizeof(ArrSinhVien[idx].Name) - 1] = '\0';
    vietHoaTatCa(ArrSinhVien[idx].Name);
    ArrSinhVien[idx].NgaySinh.day = d;
    ArrSinhVien[idx].NgaySinh.month = m;
    ArrSinhVien[idx].NgaySinh.year = y;
    strncpy(ArrSinhVien[idx].Gender, gender, sizeof(ArrSinhVien[idx].Gender) - 1);
    ArrSinhVien[idx].Gender[sizeof(ArrSinhVien[idx].Gender) - 1] = '\0';
    strncpy(ArrSinhVien[idx].Address, addrBuf, sizeof(ArrSinhVien[idx].Address) - 1);
    ArrSinhVien[idx].Address[sizeof(ArrSinhVien[idx].Address) - 1] = '\0';
    vietHoaChuDau(ArrSinhVien[idx].Address);
    strncpy(ArrSinhVien[idx].Class, classBuf, sizeof(ArrSinhVien[idx].Class) - 1);
    ArrSinhVien[idx].Class[sizeof(ArrSinhVien[idx].Class) - 1] = '\0';
    strncpy(ArrSinhVien[idx].Email, emailBuf, sizeof(ArrSinhVien[idx].Email) - 1);
    ArrSinhVien[idx].Email[sizeof(ArrSinhVien[idx].Email) - 1] = '\0';

    // Persist and refresh
    LuuVaoFile(ArrSinhVien, slsv);
    DaSapXep = 0;
    refresh_delete_list(ed->app);

    GtkAlertDialog *ok = gtk_alert_dialog_new("Sửa thành công");
    gtk_alert_dialog_set_detail(ok, "Đã cập nhật thông tin sinh viên.");
    gtk_alert_dialog_set_buttons(ok, (const char *[]){"OK", NULL});
    gtk_alert_dialog_show(ok, GTK_WINDOW(gtk_widget_get_root(ed->window)));

    // Close dialog
    gtk_window_destroy(GTK_WINDOW(ed->window));
    g_free(ed);
}

static void on_edit_clicked(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    int selectedIndex = get_selected_delete_index(app);
    if (selectedIndex < 0 || selectedIndex >= slsv) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("⚠️ Lỗi");
        gtk_alert_dialog_set_detail(dialog, "Vui lòng chọn một sinh viên từ kết quả để sửa.");
        gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
        return;
    }

    EditDialogData *ed = g_new0(EditDialogData, 1);
    ed->app = app;
    ed->index = selectedIndex;

    GtkWidget *dialog = gtk_window_new();
    ed->window = dialog;
    gtk_window_set_title(GTK_WINDOW(dialog), "Sửa thông tin sinh viên");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(gtk_widget_get_root(app->stack)));
    gtk_window_set_default_size(GTK_WINDOW(dialog), 480, 360);

    GtkWidget *content = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_widget_set_margin_top(content, 12);
    gtk_widget_set_margin_bottom(content, 12);
    gtk_widget_set_margin_start(content, 12);
    gtk_widget_set_margin_end(content, 12);
    gtk_window_set_child(GTK_WINDOW(dialog), content);

    GtkWidget *row_name = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *lbl_name = gtk_label_new("Tên đầy đủ:");
    gtk_widget_set_halign(lbl_name, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(row_name), lbl_name);
    ed->entryName = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(ed->entryName), ArrSinhVien[selectedIndex].Name);
    gtk_box_append(GTK_BOX(row_name), ed->entryName);
    gtk_box_append(GTK_BOX(content), row_name);

    GtkWidget *row_date = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *lbl_date = gtk_label_new("Ngày sinh (DD/MM/YYYY):");
    gtk_widget_set_halign(lbl_date, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(row_date), lbl_date);
    ed->entryDate = gtk_entry_new();
    char datebuf[32];
    g_snprintf(datebuf, sizeof(datebuf), "%02d/%02d/%04d", ArrSinhVien[selectedIndex].NgaySinh.day, ArrSinhVien[selectedIndex].NgaySinh.month, ArrSinhVien[selectedIndex].NgaySinh.year);
    gtk_editable_set_text(GTK_EDITABLE(ed->entryDate), datebuf);
    gtk_box_append(GTK_BOX(row_date), ed->entryDate);
    gtk_box_append(GTK_BOX(content), row_date);

    GtkWidget *row_gender = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *lbl_gender = gtk_label_new("Giới tính:");
    gtk_widget_set_halign(lbl_gender, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(row_gender), lbl_gender);
    ed->entryGender = gtk_drop_down_new_from_strings((const char * const[]){"", "Nam", "Nữ", NULL});
    if (g_strcmp0(ArrSinhVien[selectedIndex].Gender, "Nam") == 0) gtk_drop_down_set_selected(GTK_DROP_DOWN(ed->entryGender), 1);
    else if (g_strcmp0(ArrSinhVien[selectedIndex].Gender, "Nữ") == 0) gtk_drop_down_set_selected(GTK_DROP_DOWN(ed->entryGender), 2);
    gtk_box_append(GTK_BOX(row_gender), ed->entryGender);
    gtk_box_append(GTK_BOX(content), row_gender);

    GtkWidget *row_address = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *lbl_address = gtk_label_new("Địa chỉ:");
    gtk_widget_set_halign(lbl_address, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(row_address), lbl_address);
    ed->entryAddress = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(ed->entryAddress), ArrSinhVien[selectedIndex].Address);
    gtk_box_append(GTK_BOX(row_address), ed->entryAddress);
    gtk_box_append(GTK_BOX(content), row_address);

    GtkWidget *row_class = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *lbl_class = gtk_label_new("Lớp:");
    gtk_widget_set_halign(lbl_class, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(row_class), lbl_class);
    ed->entryClass = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(ed->entryClass), ArrSinhVien[selectedIndex].Class);
    gtk_editable_set_editable(GTK_EDITABLE(ed->entryClass), FALSE);
    gtk_widget_add_css_class(ed->entryClass, "readonly-entry");
    gtk_box_append(GTK_BOX(row_class), ed->entryClass);
    gtk_box_append(GTK_BOX(content), row_class);

    GtkWidget *row_email = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *lbl_email = gtk_label_new("Email:");
    gtk_widget_set_halign(lbl_email, GTK_ALIGN_START);
    gtk_box_append(GTK_BOX(row_email), lbl_email);
    ed->entryEmail = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(ed->entryEmail), ArrSinhVien[selectedIndex].Email);
    gtk_editable_set_editable(GTK_EDITABLE(ed->entryEmail), FALSE);
    gtk_widget_add_css_class(ed->entryEmail, "readonly-entry");
    gtk_box_append(GTK_BOX(row_email), ed->entryEmail);
    gtk_box_append(GTK_BOX(content), row_email);

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    GtkWidget *btn_cancel = gtk_button_new_with_label("Hủy");
    GtkWidget *btn_save = gtk_button_new_with_label("Lưu");
    g_signal_connect_swapped(btn_cancel, "clicked", G_CALLBACK(gtk_window_destroy), dialog);
    g_signal_connect(btn_save, "clicked", G_CALLBACK(on_edit_save), ed);
    gtk_box_append(GTK_BOX(button_box), btn_cancel);
    gtk_box_append(GTK_BOX(button_box), btn_save);
    gtk_box_append(GTK_BOX(content), button_box);

    gtk_window_present(GTK_WINDOW(dialog));
}

static void on_reset_delete_view(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    if (app->deleteEntry)
        gtk_editable_set_text(GTK_EDITABLE(app->deleteEntry), "");
    gtk_label_set_text(GTK_LABEL(app->deleteMsg), "");
    refresh_delete_list(app);
}

void on_show_list_by_name_and_mssv(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    const char *string = gtk_editable_get_text(GTK_EDITABLE(app->deleteEntry));
    if (strlen(string) == 0) {
        on_reset_delete_view(NULL, app);
        return;
    }
    
    // Xóa danh sách cũ
    GtkWidget *child = gtk_widget_get_first_child(app->deleteListBox);
    while (child) {
        gtk_flow_box_remove(GTK_FLOW_BOX(app->deleteListBox), child);
        child = gtk_widget_get_first_child(app->deleteListBox);
    }
    
    char name_upper[100];
    strcpy(name_upper, string);
    vietHoaTatCa(name_upper);
    int found = 0;

    if (is_all_digits(name_upper)) {
        long long maTim = atoll(name_upper);
        for (int i = 0; i < slsv; i++) {
            if (ArrSinhVien[i].MaSV == maTim) {
                GtkWidget *row = create_student_card_box(15);
                GtkWidget *icon = gtk_image_new_from_icon_name("avatar-default-symbolic");
                gtk_image_set_pixel_size(GTK_IMAGE(icon), 40);
                GtkWidget *lbl = gtk_label_new(NULL);
                char *txt = g_strdup_printf("<b>%s</b>\n<span size='medium' color='#666666'>MSSV: %lld | Lớp: %s\nGiới tính: %s | Địa chỉ: %s\nEmail: %s</span>", 
                                            ArrSinhVien[i].Name, ArrSinhVien[i].MaSV, ArrSinhVien[i].Class,
                                            ArrSinhVien[i].Gender, ArrSinhVien[i].Address, ArrSinhVien[i].Email);
                gtk_label_set_markup(GTK_LABEL(lbl), txt);
                g_free(txt);
                gtk_box_append(GTK_BOX(row), icon);
                gtk_box_append(GTK_BOX(row), lbl);
                gtk_flow_box_insert(GTK_FLOW_BOX(app->deleteListBox), row, -1);
                GtkWidget *flow_child = gtk_widget_get_parent(row);
                if (flow_child) {
                    g_object_set_data(G_OBJECT(flow_child), "student-index", GINT_TO_POINTER(i + 1));
                }
                found = 1;
                break;
            }
        }
    } else {
        for (int i = 0; i < slsv; i++) {
            if (strstr(ArrSinhVien[i].Name, name_upper) != NULL) {
                GtkWidget *row = create_student_card_box(15);
                GtkWidget *icon = gtk_image_new_from_icon_name("avatar-default-symbolic");
                gtk_image_set_pixel_size(GTK_IMAGE(icon), 40);
                GtkWidget *lbl = gtk_label_new(NULL);
                char *txt = g_strdup_printf("<b>%s</b>\n<span size='medium' color='#666666'>MSSV: %lld | Lớp: %s\nGiới tính: %s | Địa chỉ: %s\nEmail: %s</span>", 
                                            ArrSinhVien[i].Name, ArrSinhVien[i].MaSV, ArrSinhVien[i].Class,
                                            ArrSinhVien[i].Gender, ArrSinhVien[i].Address, ArrSinhVien[i].Email);
                gtk_label_set_markup(GTK_LABEL(lbl), txt);
                gtk_label_set_xalign(GTK_LABEL(lbl), 0);
                g_free(txt);
                gtk_box_append(GTK_BOX(row), icon);
                gtk_box_append(GTK_BOX(row), lbl);
                gtk_flow_box_insert(GTK_FLOW_BOX(app->deleteListBox), row, -1);
                GtkWidget *flow_child = gtk_widget_get_parent(row);
                if (flow_child) {
                    gtk_widget_set_size_request(flow_child, 350, 120);
                    g_object_set_data(G_OBJECT(flow_child), "student-index", GINT_TO_POINTER(i + 1));
                }
                found = 1;
            }
        }
    }
    
    if (!found) {
        GtkWidget *row = create_student_card_box(10);
        GtkWidget *lbl = gtk_label_new(NULL);
        char *smg = g_strdup_printf("<span size = 'large' weight = 'bold' color = 'red'> ❌ Không tìm thấy sinh viên nào! </span>");
        gtk_label_set_markup(GTK_LABEL(lbl), smg);
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->deleteListBox), row, -1);
    }
    
    gtk_label_set_text(GTK_LABEL(app->deleteMsg), "");
    app->selectedDeleteIndex = -1;
}

GtkWidget* create_delete_sv_ui(SinhvienWidgets *w) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    set_margin_all(vbox, 20);

    // Icon + Header
    GtkWidget *icon_header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(icon_header_box, GTK_ALIGN_CENTER);
    GtkWidget *icon = gtk_image_new_from_file(REMOVE_ICON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 120);
    gtk_box_append(GTK_BOX(icon_header_box), icon);
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='xx-large' weight='bold'>QUẢN LÝ HỒ SƠ SINH VIÊN</span>");
    gtk_box_append(GTK_BOX(icon_header_box), header);
    gtk_widget_set_margin_bottom(icon_header_box, 20);
    gtk_box_append(GTK_BOX(vbox), icon_header_box);

    // Instructions
    GtkWidget *instr = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(instr), "<span size='x-large' weight='bold' color='#fffb00'>Chọn sinh viên từ bảng kết quả để Tra cứu, sửa đổi hoặc cập nhật hồ sơ sinh viên theo Tên / MSSV.</span>");
    gtk_label_set_xalign(GTK_LABEL(instr), 0.5);
    gtk_box_append(GTK_BOX(vbox), instr);

    // Delete by name / MSSV
    GtkWidget *form_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_halign(form_box, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(form_box, TRUE);
    gtk_widget_set_margin_top(form_box, 10);

    // ============== box upset
    GtkWidget *left_name_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    // Back button
    GtkWidget *btn_back = gtk_button_new_with_label("◀ Quay lại Menu");
    gtk_widget_add_css_class(btn_back, "label-btn");
    gtk_widget_add_css_class(btn_back, "border-label");
    gtk_widget_set_size_request(btn_back, 160, 50);
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_clicked), w);
    gtk_widget_set_margin_top(btn_back, 10);
    gtk_widget_add_css_class(btn_back, "back-button");
    gtk_box_append(GTK_BOX(left_name_box), btn_back);

    GtkWidget *center_name_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *name_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(name_label), "<span size = 'large' weight = 'bold'>  Tìm theo Họ và Tên / MSSV:     \n(Nhấn Enter để reset danh sách) </span>");
    gtk_widget_set_size_request(name_label, 180, -1);
    w->deleteEntry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(w->deleteEntry), "Nhập Tên đầy đủ / MSSV và nhấn Enter");
    gtk_widget_set_size_request(w->deleteEntry, 260, 35);
    g_signal_connect(w->deleteEntry, "activate", G_CALLBACK(on_show_list_by_name_and_mssv), w);
    gtk_box_append(GTK_BOX(center_name_box), name_label);
    gtk_box_append(GTK_BOX(center_name_box), w->deleteEntry);

    GtkWidget *right_name_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *btn_edit_selected = gtk_button_new_with_label("SỬA MỤC ĐÃ CHỌN");
    gtk_widget_add_css_class(btn_edit_selected, "label-btn");
    gtk_widget_add_css_class(btn_edit_selected, "border-label");
    gtk_widget_set_size_request(btn_edit_selected, 160, 50);
    g_signal_connect(btn_edit_selected, "clicked", G_CALLBACK(on_edit_clicked), w);
    gtk_box_append(GTK_BOX(right_name_box), btn_edit_selected);
    
    // name find box
    GtkWidget *name_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(name_box, TRUE);

    gtk_box_append(GTK_BOX(name_box), left_name_box);
    
    GtkWidget *spacer_name1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer_name1, TRUE);
    gtk_box_append(GTK_BOX(name_box), spacer_name1);

    gtk_box_append(GTK_BOX(name_box), center_name_box);

    GtkWidget *spacer_name2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer_name2, TRUE);
    gtk_box_append(GTK_BOX(name_box), spacer_name2);

    gtk_box_append(GTK_BOX(name_box), right_name_box);

    gtk_box_append(GTK_BOX(form_box), name_box);
    ///=========================
    
    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(form_box), separator);
    
    // =================box downset
    // GtkWidget *center_mssv_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    // GtkWidget *mssv_label = gtk_label_new(NULL);
    // gtk_label_set_markup(GTK_LABEL(mssv_label), "<span size = 'large' weight = 'bold'>Tìm theo MSSV : </span>");
    // gtk_widget_set_size_request(mssv_label, 180, -1);
    // w->deleteMSSVEntry = gtk_entry_new();
    // gtk_entry_set_placeholder_text(GTK_ENTRY(w->deleteMSSVEntry), "Nhập MSSV và nhấn Enter");
    // gtk_widget_set_size_request(w->deleteMSSVEntry, 260, 35);
    // g_signal_connect(w->deleteMSSVEntry, "activate", G_CALLBACK(on_show_list_by_mssv), w);
    // gtk_box_append(GTK_BOX(center_mssv_box), mssv_label);
    // gtk_box_append(GTK_BOX(center_mssv_box), w->deleteMSSVEntry);

    GtkWidget *spacer_mssv1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer_mssv1, TRUE);
    GtkWidget *spacer_mssv2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer_mssv2, TRUE);
    
    GtkWidget *right_mssv_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *btn_delete = gtk_button_new_with_label("XÓA MỤC ĐÃ CHỌN");
    gtk_widget_add_css_class(btn_delete, "label-btn");
    gtk_widget_add_css_class(btn_delete, "border-label");
    gtk_widget_set_size_request(btn_delete, 160, 50);
    g_signal_connect(btn_delete, "clicked", G_CALLBACK(on_delete_clicked), w);
    gtk_box_append(GTK_BOX(right_mssv_box), btn_delete);
    
    // mssv find box
    GtkWidget *mssv_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(mssv_box, TRUE);
    
    gtk_box_append(GTK_BOX(mssv_box), spacer_mssv1);
    // gtk_box_append(GTK_BOX(mssv_box), center_mssv_box);
    gtk_box_append(GTK_BOX(mssv_box), spacer_mssv2);
    gtk_box_append(GTK_BOX(mssv_box), right_mssv_box);

    gtk_box_append(GTK_BOX(form_box), mssv_box);
    //=========================
    gtk_box_append(GTK_BOX(vbox), form_box);

    // Search results - LIST SELECTION
    GtkWidget *search_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(search_label), "<span size='large' weight='bold' color='black'>Danh sách sinh viên:</span>");
    gtk_label_set_xalign(GTK_LABEL(search_label), 0);
    gtk_widget_set_margin_top(search_label, 10);
    gtk_box_append(GTK_BOX(vbox), search_label);

    GtkWidget *search_scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(search_scrolled, -1, 250);
    gtk_widget_set_vexpand(search_scrolled, TRUE);

    w->deleteListBox = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(w->deleteListBox), 5);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(w->deleteListBox), 1);
    gtk_flow_box_set_homogeneous(GTK_FLOW_BOX(w->deleteListBox), FALSE);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(w->deleteListBox), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(w->deleteListBox), 12);
    gtk_widget_set_halign(w->deleteListBox, GTK_ALIGN_FILL);
    // gtk_widget_set_hexpand(w->deleteListBox, TRUE);
    gtk_flow_box_set_selection_mode(GTK_FLOW_BOX(w->deleteListBox), GTK_SELECTION_SINGLE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(search_scrolled), w->deleteListBox);
    gtk_box_append(GTK_BOX(vbox), search_scrolled);

    w->deleteMsg = gtk_label_new("");
    gtk_widget_set_margin_top(w->deleteMsg, 10);
    gtk_box_append(GTK_BOX(vbox), w->deleteMsg);

    return vbox;
}

/// ======= SẮP XẾP SINH VIÊN =======
void refresh_sort_list(SinhvienWidgets *app) {
    GtkWidget *child = gtk_widget_get_first_child(app->sortListBox);
    while (child) {
        gtk_flow_box_remove(GTK_FLOW_BOX(app->sortListBox), child);
        child = gtk_widget_get_first_child(app->sortListBox);
    }
    
    gtk_label_set_text(GTK_LABEL(app->sortMsg), "");

    if (slsv == 0) {
        GtkWidget *row = create_student_card_box(10);
        GtkWidget *lbl = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl), "<span size='large' weight='bold' color='#ff0000'>❌ Danh sách sinh viên đang trống!</span>");
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->sortListBox), row, -1);
        return;
    }

    for (int i = 0; i < slsv; i++) {
        GtkWidget *row = create_student_card_box(15);

        GtkWidget *icon = gtk_image_new_from_icon_name("avatar-default-symbolic");
        gtk_image_set_pixel_size(GTK_IMAGE(icon), 40);

        GtkWidget *lbl = gtk_label_new(NULL);
        char mssv_text[64];
        if (ArrSinhVien[i].MaSV != 0) {
            g_snprintf(mssv_text, sizeof(mssv_text), "MSSV: %lld", ArrSinhVien[i].MaSV);
        } else {
            g_snprintf(mssv_text, sizeof(mssv_text), "MSSV: Chưa có");
        }
        char *txt = g_strdup_printf("<b>%s</b>\n<span size='medium' color='#666666'>%s | Lớp: %s | Ngày sinh: %d/%d/%d</span>", 
                                    ArrSinhVien[i].Name, mssv_text, ArrSinhVien[i].Class, ArrSinhVien[i].NgaySinh.day, ArrSinhVien[i].NgaySinh.month, ArrSinhVien[i].NgaySinh.year);
        gtk_label_set_markup(GTK_LABEL(lbl), txt);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0);
        g_free(txt);

        gtk_box_append(GTK_BOX(row), icon);
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->sortListBox), row, -1);
    }
}

void refresh_cap_list(SinhvienWidgets *app) {
    GtkWidget *child = gtk_widget_get_first_child(app->capListBox);
    while (child) {
        gtk_flow_box_remove(GTK_FLOW_BOX(app->capListBox), child);
        child = gtk_widget_get_first_child(app->capListBox);
    }
    
    gtk_label_set_text(GTK_LABEL(app->capMsg), "");

    if (slsv == 0) {
        GtkWidget *row = create_student_card_box(10);
        GtkWidget *lbl = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl), "<span size='large' weight='bold' color='#ff0000'>❌ Danh sách sinh viên đang trống!</span>");
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->capListBox), row, -1);
        return;
    }

    for (int i = 0; i < slsv; i++) {
        GtkWidget *row = create_student_card_box(15);

        GtkWidget *icon = gtk_image_new_from_icon_name("avatar-default-symbolic");
        gtk_image_set_pixel_size(GTK_IMAGE(icon), 40);

        GtkWidget *lbl = gtk_label_new(NULL);
        const char *status = ArrSinhVien[i].MaSV != 0 ? "✅ Đã cấp MSSV" : "○ Chưa cấp MSSV";
        char *txt = g_strdup_printf("<b>%s</b>\n<span size='medium' color='#666666'>MSSV: %s | Lớp: %s</span>", 
                                    ArrSinhVien[i].Name, status, ArrSinhVien[i].Class);
        gtk_label_set_markup(GTK_LABEL(lbl), txt);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0);
        g_free(txt);

        gtk_box_append(GTK_BOX(row), icon);
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->capListBox), row, -1);
    }
}

void refresh_cap_email_list(SinhvienWidgets *app) {
    GtkWidget *child = gtk_widget_get_first_child(app->capEmailListBox);
    while (child) {
        gtk_flow_box_remove(GTK_FLOW_BOX(app->capEmailListBox), child);
        child = gtk_widget_get_first_child(app->capEmailListBox);
    }
    
    gtk_label_set_text(GTK_LABEL(app->capEmailMsg), "");

    if (slsv == 0) {
        GtkWidget *row = create_student_card_box(10);
        GtkWidget *lbl = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(lbl), "<span size='large' weight='bold' color='#ff0000'>❌ Danh sách sinh viên đang trống!</span>");
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->capEmailListBox), row, -1);
        return;
    }

    for (int i = 0; i < slsv; i++) {
        GtkWidget *row = create_student_card_box(15);

        GtkWidget *icon = gtk_image_new_from_icon_name("avatar-default-symbolic");
        gtk_image_set_pixel_size(GTK_IMAGE(icon), 40);

        GtkWidget *lbl = gtk_label_new(NULL);
        const char *email_status = strcmp(ArrSinhVien[i].Email, "Chưa có") != 0 ? "✓ Đã cấp Email" : "○ Chưa cấp Email";
        char *txt = g_strdup_printf("<b>%s</b>\n<span size='medium' color='#666666'>MSSV: %lld | Năm sinh: %d | Email: %s</span>", 
                                    ArrSinhVien[i].Name, ArrSinhVien[i].MaSV, ArrSinhVien[i].NgaySinh.year, email_status);
        gtk_label_set_markup(GTK_LABEL(lbl), txt);
        gtk_label_set_xalign(GTK_LABEL(lbl), 0);
        g_free(txt);

        gtk_box_append(GTK_BOX(row), icon);
        gtk_box_append(GTK_BOX(row), lbl);
        gtk_flow_box_insert(GTK_FLOW_BOX(app->capEmailListBox), row, -1);
    }
}

void on_go_sort(GtkWidget *w, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    refresh_sort_list(app);
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "sort");
}

void on_sort_clicked(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;

    if (slsv == 0) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("⚠️ Lỗi");
        gtk_alert_dialog_set_detail(dialog, "⚠️ Danh sách sinh viên đang trống!");
        gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
        return;
    }

    int criteria = gtk_drop_down_get_selected(GTK_DROP_DOWN(app->sortCriteriaDropDown));
    switch (criteria) {
        case 0:
            SapXepTheoLop(ArrSinhVien, slsv);
            break;
        case 1:
            SapXepTheoNgaySinh(ArrSinhVien, slsv);
            break;
        case 2:
            SapXepTheoMSSV(ArrSinhVien, slsv);
            break;
        case 3:
            SapXepTheoTen(ArrSinhVien, slsv);
            DaSapXep = 1;
            break;
        default:
            SapXepTheoTen(ArrSinhVien, slsv);
            DaSapXep = 1;
            break;
    }

    LuuVaoFile(ArrSinhVien, slsv);

    GtkAlertDialog *success_dialog = gtk_alert_dialog_new("🎉 Thành công");
    gtk_alert_dialog_set_detail(success_dialog, "✅ Đã sắp xếp sinh viên thành công!");
    gtk_alert_dialog_set_buttons(success_dialog, (const char *[]){"OK", NULL});
    gtk_alert_dialog_show(success_dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
    refresh_sort_list(app);
}

void on_cap_mssv_clicked(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    
    if (slsv == 0) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("⚠️ Lỗi");
        gtk_alert_dialog_set_detail(dialog, "⚠️ Danh sách sinh viên đang trống!");
        gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
        return;
    }
    
    if (DaSapXep == 0) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("⚠️ Lỗi");
        gtk_alert_dialog_set_detail(dialog, "⚠️ Chưa sắp xếp danh sách theo Tên! Vui lòng chọn chức năng sắp xếp theo Tên trước khi cấp MSSV.");
        gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
        return;
    }
    
    CapMaSoSinhVien(ArrSinhVien, slsv);
    LuuVaoFile(ArrSinhVien, slsv);
    
    GtkAlertDialog *success_dialog = gtk_alert_dialog_new("🎉 Thành công");
    gtk_alert_dialog_set_detail(success_dialog, "✅ Đã cấp MSSV thành công!");
    gtk_alert_dialog_set_buttons(success_dialog, (const char *[]){"OK", NULL});
    gtk_alert_dialog_show(success_dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
    refresh_cap_list(app);
}

void on_cap_email_clicked(GtkWidget *widget, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    
    if (slsv == 0) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("⚠️ Lỗi");
        gtk_alert_dialog_set_detail(dialog, "⚠️ Danh sách sinh viên đang trống!");
        gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
        return;
    }
    
    int hasUnassigned = 0;
    for (int i = 0; i < slsv; i++) {
        if (ArrSinhVien[i].MaSV == 0) {
            hasUnassigned = 1;
            break;
        }
    }
    
    if (hasUnassigned) {
        GtkAlertDialog *dialog = gtk_alert_dialog_new("⚠️ Lỗi");
        gtk_alert_dialog_set_detail(dialog, "⚠️ Phải cấp MSSV cho tất cả sinh viên trước khi cấp Email!");
        gtk_alert_dialog_set_buttons(dialog, (const char *[]){"OK", NULL});
        gtk_alert_dialog_show(dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
        return;
    }
    
    CapEmailSinhVien(ArrSinhVien, slsv);
    LuuVaoFile(ArrSinhVien, slsv);
    
    GtkAlertDialog *success_dialog = gtk_alert_dialog_new("🎉 Thành công");
    gtk_alert_dialog_set_detail(success_dialog, "✅ Đã cấp Email thành công!");
    gtk_alert_dialog_set_buttons(success_dialog, (const char *[]){"OK", NULL});
    gtk_alert_dialog_show(success_dialog, GTK_WINDOW(gtk_widget_get_root(widget)));
    refresh_cap_email_list(app);
}

GtkWidget* create_sort_sv_ui(SinhvienWidgets *w) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    set_margin_all(vbox, 20);

    // Icon + Header
    GtkWidget *icon_header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(icon_header_box, GTK_ALIGN_CENTER);
    GtkWidget *icon = gtk_image_new_from_file(SORT_ICON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 48);
    gtk_box_append(GTK_BOX(icon_header_box), icon);
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='xx-large' weight='bold'>SẮP XẾP SINH VIÊN</span>");
    gtk_box_append(GTK_BOX(icon_header_box), header);
    gtk_widget_set_margin_bottom(icon_header_box, 20);
    gtk_box_append(GTK_BOX(vbox), icon_header_box);

    // Instructions
    GtkWidget *instr = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(instr), "<span size='x-large' weight='bold' color='#fffb00'>Sắp xếp danh sách theo tiêu chí được chọn.</span>");
    gtk_label_set_xalign(GTK_LABEL(instr), 0.5);
    gtk_box_append(GTK_BOX(vbox), instr);
    
    // Sort criteria selector
    GtkWidget *criteria_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_top(criteria_box, 10);
    gtk_widget_set_margin_bottom(criteria_box, 6);
    gtk_widget_set_hexpand(criteria_box, TRUE);

    // Back button
    GtkWidget *btn_back = gtk_button_new_with_label("◀ Quay lại Menu");
    gtk_widget_add_css_class(btn_back, "label-btn");
    gtk_widget_add_css_class(btn_back, "border-label");
    gtk_widget_set_size_request(btn_back, 250, 50);
    gtk_widget_set_margin_top(btn_back, 10);
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_clicked), w);
    gtk_widget_add_css_class(btn_back, "back-button");
    gtk_box_append(GTK_BOX(criteria_box), btn_back);

    // spacer_fill
    GtkWidget *spacer1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer1, TRUE);
    gtk_box_append(GTK_BOX(criteria_box), spacer1);

    GtkWidget *criteria_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(criteria_label), "<span weight='bold' size='large' color='#000000'>Tiêu chí sắp xếp:</span>");
    gtk_widget_set_halign(criteria_label, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(criteria_box), criteria_label);

    w->sortCriteriaDropDown = gtk_drop_down_new_from_strings((const char * const[]){"Theo lớp", "Theo ngày sinh", "Theo MSSV", "Theo tên", NULL});
    gtk_widget_set_hexpand(w->sortCriteriaDropDown, FALSE);
    gtk_widget_set_size_request(w->sortCriteriaDropDown, 180, 50);
    gtk_box_append(GTK_BOX(criteria_box), w->sortCriteriaDropDown);

    GtkWidget *spacer2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer2, TRUE);
    gtk_box_append(GTK_BOX(criteria_box), spacer2);

    // Sort button
    GtkWidget *btn_sort = gtk_button_new_with_label("SẮP XẾP");
    gtk_widget_add_css_class(btn_sort, "label-btn");
    gtk_widget_add_css_class(btn_sort, "border-label");
    gtk_widget_set_size_request(btn_sort, 250, 50);
    gtk_widget_set_margin_top(btn_sort, 10);
    g_signal_connect(btn_sort, "clicked", G_CALLBACK(on_sort_clicked), w);
    gtk_box_append(GTK_BOX(criteria_box), btn_sort);

    gtk_box_append(GTK_BOX(vbox), criteria_box);

    // Student list
    GtkWidget *list_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(list_label), "<span size='large' weight='bold' color='black'>Danh sách sinh viên hiện tại:</span>");
    gtk_label_set_xalign(GTK_LABEL(list_label), 0);
    gtk_widget_set_margin_top(list_label, 15);
    gtk_box_append(GTK_BOX(vbox), list_label);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    gtk_widget_set_size_request(scrolled, -1, 300);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    w->sortListBox = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(w->sortListBox), 5);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(w->sortListBox), 1);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(w->sortListBox), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(w->sortListBox), 12);
    gtk_widget_set_halign(w->sortListBox, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(w->sortListBox, TRUE);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), w->sortListBox);
    gtk_box_append(GTK_BOX(vbox), scrolled);

    // Message label
    w->sortMsg = gtk_label_new("");
    gtk_widget_set_margin_top(w->sortMsg, 10);
    gtk_box_append(GTK_BOX(vbox), w->sortMsg);

    return vbox;
}

GtkWidget* create_cap_mssv_ui(SinhvienWidgets *w) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    set_margin_all(vbox, 20);

    // Icon + Header
    GtkWidget *icon_header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(icon_header_box, GTK_ALIGN_CENTER);
    GtkWidget *icon = gtk_image_new_from_file(MSSV_ICON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 48);
    gtk_box_append(GTK_BOX(icon_header_box), icon);
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='xx-large' weight='bold'>CẤP MÃ SỐ SINH VIÊN</span>");
    gtk_box_append(GTK_BOX(icon_header_box), header);
    gtk_widget_set_margin_bottom(icon_header_box, 20);
    gtk_box_append(GTK_BOX(vbox), icon_header_box);

    // Instructions
    GtkWidget *instr = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(instr), "<span size='x-large' weight='bold' color='#fffb00'>Chỉ cấp MSSV khi đã sắp xếp danh sách theo Tên\nMSSV = Mã Khoa + 2 số năm học + Số thứ tự trong khoa</span>");
    gtk_label_set_xalign(GTK_LABEL(instr), 0.5);
    gtk_box_append(GTK_BOX(vbox), instr);

    // ======================= Function box
    GtkWidget *func_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_top(func_box, 10);
    // Back button
    GtkWidget *btn_back = gtk_button_new_with_label("◀ Quay lại Menu");
    gtk_widget_add_css_class(btn_back, "label-btn");
    gtk_widget_add_css_class(btn_back, "border-label");
    gtk_widget_set_size_request(btn_back, 250, 50);
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_clicked), w);
    gtk_widget_set_halign(btn_back, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(btn_back, 10);
    gtk_box_append(GTK_BOX(func_box), btn_back);

    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(func_box), spacer);

    // Cap MSSV button
    GtkWidget *btn_cap = gtk_button_new_with_label("CẤP MSSV");
    gtk_widget_add_css_class(btn_cap, "label-btn");
    gtk_widget_add_css_class(btn_cap, "border-label");
    gtk_widget_set_size_request(btn_cap, 250, 50);
    gtk_widget_set_margin_top(btn_cap, 10);
    gtk_widget_set_halign(btn_cap, GTK_ALIGN_CENTER);
    g_signal_connect(btn_cap, "clicked", G_CALLBACK(on_cap_mssv_clicked), w);
    gtk_box_append(GTK_BOX(func_box), btn_cap);

    gtk_box_append(GTK_BOX(vbox), func_box);
    // =======================

    // Student list
    GtkWidget *list_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(list_label), "<span size='large' weight='bold' color='black'>Danh sách sinh viên hiện tại:</span>");
    gtk_label_set_xalign(GTK_LABEL(list_label), 0);
    gtk_widget_set_margin_top(list_label, 15);
    gtk_box_append(GTK_BOX(vbox), list_label);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    // FIX LỖI 1: Dùng size_request kết hợp vexpand để giữ nút ở dưới cùng cửa sổ
    gtk_widget_set_size_request(scrolled, -1, 300);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    w->capListBox = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(w->capListBox), 5);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(w->capListBox), 1);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(w->capListBox), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(w->capListBox), 12);
    gtk_widget_set_halign(w->capListBox, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(w->capListBox, TRUE);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), w->capListBox);
    gtk_box_append(GTK_BOX(vbox), scrolled);

    // Message label
    w->capMsg = gtk_label_new("");
    gtk_widget_set_margin_top(w->capMsg, 10);
    gtk_box_append(GTK_BOX(vbox), w->capMsg);

    return vbox;
}

GtkWidget* create_cap_email_ui(SinhvienWidgets *w) {
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    set_margin_all(vbox, 20);

    // Icon + Header
    GtkWidget *icon_header_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(icon_header_box, GTK_ALIGN_CENTER);
    GtkWidget *icon = gtk_image_new_from_file(EMAIL_ICON);
    gtk_image_set_pixel_size(GTK_IMAGE(icon), 48);
    gtk_box_append(GTK_BOX(icon_header_box), icon);
    
    GtkWidget *header = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(header), "<span size='xx-large' weight='bold'>CẤP EMAIL SINH VIÊN</span>");
    gtk_box_append(GTK_BOX(icon_header_box), header);
    gtk_widget_set_margin_bottom(icon_header_box, 20);
    gtk_box_append(GTK_BOX(vbox), icon_header_box);

    // Instructions
    GtkWidget *instr = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(instr), "<span size='x-large' weight='bold' color='#fffb00'>Chỉ cấp Email khi đã có MSSV\nEmail = MSSV@dut.udn.vn</span>");
    gtk_label_set_xalign(GTK_LABEL(instr), 0.5);
    gtk_box_append(GTK_BOX(vbox), instr);

    // ======================= Function box
    GtkWidget *func_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_widget_set_margin_top(func_box, 10);
    // Back button
    GtkWidget *btn_back = gtk_button_new_with_label("◀ Quay lại Menu");
    gtk_widget_add_css_class(btn_back, "label-btn");
    gtk_widget_add_css_class(btn_back, "border-label");
    gtk_widget_set_size_request(btn_back, 250, 50);
    g_signal_connect(btn_back, "clicked", G_CALLBACK(on_back_clicked), w);
    gtk_widget_set_halign(btn_back, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(btn_back, 10);
    gtk_box_append(GTK_BOX(func_box), btn_back);

    GtkWidget *spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_box_append(GTK_BOX(func_box), spacer);

    // Cap Email button
    GtkWidget *btn_cap = gtk_button_new_with_label("CẤP EMAIL");
    gtk_widget_add_css_class(btn_cap, "label-btn");
    gtk_widget_add_css_class(btn_cap, "border-label");
    gtk_widget_set_size_request(btn_cap, 250, 50);
    gtk_widget_set_margin_top(btn_cap, 10);
    gtk_widget_set_halign(btn_cap, GTK_ALIGN_CENTER);
    g_signal_connect(btn_cap, "clicked", G_CALLBACK(on_cap_email_clicked), w);
    gtk_box_append(GTK_BOX(func_box), btn_cap);

    gtk_box_append(GTK_BOX(vbox), func_box);
    // =======================

    // Student list
    GtkWidget *list_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(list_label), "<span size='large' weight='bold' color='black'>Danh sách sinh viên hiện tại:</span>");
    gtk_label_set_xalign(GTK_LABEL(list_label), 0);
    gtk_widget_set_margin_top(list_label, 15);
    gtk_box_append(GTK_BOX(vbox), list_label);

    GtkWidget *scrolled = gtk_scrolled_window_new();
    // FIX LỖI 1: Dùng size_request kết hợp vexpand để giữ nút ở dưới cùng cửa sổ
    gtk_widget_set_size_request(scrolled, -1, 300);
    gtk_widget_set_vexpand(scrolled, TRUE);
    
    w->capEmailListBox = gtk_flow_box_new();
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(w->capEmailListBox), 5);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(w->capEmailListBox), 1);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(w->capEmailListBox), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(w->capEmailListBox), 12);
    gtk_widget_set_halign(w->capEmailListBox, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand(w->capEmailListBox, TRUE);
    
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), w->capEmailListBox);
    gtk_box_append(GTK_BOX(vbox), scrolled);

    // Message label
    w->capEmailMsg = gtk_label_new("");
    gtk_widget_set_margin_top(w->capEmailMsg, 10);
    gtk_box_append(GTK_BOX(vbox), w->capEmailMsg);

    return vbox;
}

/// ======= CẤP MSSV =======
void on_go_cap_mssv(GtkWidget *w, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    refresh_cap_list(app);
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "cap_mssv");
}

/// ======= CẤP EMAIL =======
void on_go_cap_email(GtkWidget *w, gpointer data) {
    SinhvienWidgets *app = (SinhvienWidgets *)data;
    refresh_cap_email_list(app);
    gtk_stack_set_visible_child_name(GTK_STACK(app->stack), "cap_email");
}

/// ======= GIAO DIỆN MENU =======
GtkWidget* create_menu_ui(SinhvienWidgets *widgets) {

    // CENTER
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_widget_set_halign(vbox, GTK_ALIGN_CENTER);
    // FIX: Đổi GTK_ALIGN_CENTER → GTK_ALIGN_START để nội dung bắt đầu từ trên
    // GTK_ALIGN_CENTER đẩy toàn bộ vbox xuống giữa window → tạo khoảng trắng lớn phía trên
    gtk_widget_set_valign(vbox, GTK_ALIGN_START);
    set_margin_all(vbox, 20); // Thêm margin để không sát mép trên

    GtkWidget *btn_add = gtk_button_new_with_label("➕ Thêm Sinh Viên");
    GtkWidget *btn_list = gtk_button_new_with_label("📋 Danh Sách Sinh Viên");
    // GtkWidget *btn_search = gtk_button_new_with_label("🔎 Tìm kiếm Sinh Viên");
    GtkWidget *btn_delete = gtk_button_new_with_label("📝 Quản Lý Hồ Sơ Sinh Viên");
    GtkWidget *btn_sort = gtk_button_new_with_label("📶 Sắp xếp Sinh Viên");
    GtkWidget *btn_cap_mssv = gtk_button_new_with_label("🆔 Cấp MSSV Tự Động Cho Sinh Viên");
    GtkWidget *btn_cap_email = gtk_button_new_with_label("📧 Cấp Email Tự Động Cho Sinh Viên");
    
    gtk_widget_add_css_class(btn_add, "label-btn");
    gtk_widget_add_css_class(btn_add, "border-label");
    gtk_widget_add_css_class(btn_list, "label-btn");
    gtk_widget_add_css_class(btn_list, "border-label");
    gtk_widget_add_css_class(btn_delete, "label-btn");
    gtk_widget_add_css_class(btn_delete, "border-label");
    gtk_widget_add_css_class(btn_sort, "label-btn");
    gtk_widget_add_css_class(btn_sort, "border-label");
    gtk_widget_add_css_class(btn_cap_mssv, "label-btn");
    gtk_widget_add_css_class(btn_cap_mssv, "border-label");
    gtk_widget_add_css_class(btn_cap_email, "label-btn");
    gtk_widget_add_css_class(btn_cap_email, "border-label");

    gtk_widget_set_size_request(btn_add, 250, 50);
    gtk_widget_set_size_request(btn_list, 250, 50);
    // gtk_widget_set_size_request(btn_search, 250, 50);
    gtk_widget_set_size_request(btn_delete, 250, 50);
    gtk_widget_set_size_request(btn_sort, 250, 50);
    gtk_widget_set_size_request(btn_cap_mssv, 250, 50);
    gtk_widget_set_size_request(btn_cap_email, 250, 50);

    GtkWidget *btn_back_to_file = gtk_button_new_with_label("🔙 Quay lại Chọn File");
    gtk_widget_add_css_class(btn_back_to_file, "label-btn");
    gtk_widget_add_css_class(btn_back_to_file, "border-label");
    gtk_widget_set_size_request(btn_back_to_file, 250, 50);

    g_signal_connect(btn_add, "clicked", G_CALLBACK(on_go_add), widgets);
    g_signal_connect(btn_list, "clicked", G_CALLBACK(on_go_list), widgets);
    // g_signal_connect(btn_search, "clicked", G_CALLBACK(on_go_search), widgets);
    g_signal_connect(btn_delete, "clicked", G_CALLBACK(on_go_delete), widgets);
    g_signal_connect(btn_sort, "clicked", G_CALLBACK(on_go_sort), widgets);
    g_signal_connect(btn_cap_mssv, "clicked", G_CALLBACK(on_go_cap_mssv), widgets);
    g_signal_connect(btn_cap_email, "clicked", G_CALLBACK(on_go_cap_email), widgets);
    g_signal_connect(btn_back_to_file, "clicked", G_CALLBACK(on_go_file_select), widgets);

    // Icon + tiêu đề chính (append trước để hiện trên cùng)
    GtkWidget *menu_header = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(menu_header, GTK_ALIGN_CENTER);

    GtkWidget *logo_row =
    gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 20);
    gtk_widget_set_halign(logo_row,GTK_ALIGN_CENTER);
    GtkWidget *left_icon = gtk_image_new_from_file(BK_PIC);
    GtkWidget *menu_icon = gtk_image_new_from_file(MENU_ICON);
    GtkWidget *right_icon = gtk_image_new_from_file(ITF_PIC);
    gtk_image_set_pixel_size(GTK_IMAGE(left_icon), 64); 
    gtk_image_set_pixel_size(GTK_IMAGE(menu_icon), 64); 
    gtk_image_set_pixel_size(GTK_IMAGE(right_icon), 64); 
    gtk_box_append(GTK_BOX(logo_row), left_icon);
    gtk_box_append(GTK_BOX(logo_row), menu_icon);
    gtk_box_append(GTK_BOX(logo_row), right_icon);
    gtk_box_append(GTK_BOX(menu_header), logo_row);

    GtkWidget *menu_title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(menu_title),
        "<span size='xx-large' weight='bold' foreground = '#263238'>QUẢN LÝ SINH VIÊN</span>");
    gtk_box_append(GTK_BOX(menu_header), menu_title);
    gtk_box_append(GTK_BOX(vbox), menu_header);

    // FIX: Xóa \n thừa ở đầu/cuối markup gây khoảng trắng lớn phía trên cụm text
    // Chuyển top_label xuống sau menu_header để đúng thứ tự: icon → tiêu đề → tác giả
    GtkWidget *top_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(top_label),
        "<span font_desc='Sans 14' weight='bold' foreground = '#263238'>🏫 ĐỒ ÁN LẬP TRÌNH TÍNH TOÁN\n"
        "<span size='large' weight='normal' foreground = '#424242'>Thực hiện bởi: Trần Khánh Duy &amp; Lã Trung Thực</span></span>");
    
    gtk_label_set_justify(GTK_LABEL(top_label), GTK_JUSTIFY_CENTER);
    gtk_label_set_xalign(GTK_LABEL(top_label), 0.5);
    gtk_widget_set_margin_bottom(top_label, 15);
    gtk_box_append(GTK_BOX(vbox), top_label);

    GtkWidget *menu_subtitle = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(menu_subtitle), "<span size = 'x-large' weight='bold' foreground='#fffb00'>Quản lý học viên toàn trường - từng lớp.</span>");
    gtk_widget_set_halign(menu_subtitle, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_bottom(menu_subtitle, 20);
    gtk_box_append(GTK_BOX(vbox), menu_subtitle);

    gtk_box_append(GTK_BOX(vbox), btn_add);
    gtk_box_append(GTK_BOX(vbox), btn_sort);
    // gtk_box_append(GTK_BOX(vbox), btn_search);
    gtk_box_append(GTK_BOX(vbox), btn_delete);
    gtk_box_append(GTK_BOX(vbox), btn_cap_mssv);
    gtk_box_append(GTK_BOX(vbox), btn_cap_email);
    gtk_box_append(GTK_BOX(vbox), btn_list);
    gtk_box_append(GTK_BOX(vbox), btn_back_to_file);

    //

    return vbox;
}

/// ======= ACTIVATE & MAIN =======
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "🏫 QUẢN LÝ SINH VIÊN - PBL1");
    gtk_window_set_icon_name(GTK_WINDOW(window), "applications-education");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 900);
    gtk_window_maximize(GTK_WINDOW(window));

    gtk_widget_set_size_request(window, 640, 600);
    /* Give the window a stable CSS name so we can target it explicitly */
    gtk_widget_set_name(window, "main-window");

    GtkCssProvider *css = gtk_css_provider_new();

    gtk_css_provider_load_from_string(
        css,
        "#main-window {"
        "  background-image: url('file:///C:/Users/admin/OneDrive/Desktop/PBL1/icon/BackGround.png');"
        "  background-size: cover;"
        "  background-position: center;"
        "  background-repeat: no-repeat;"
        "}"

        "#main-window .student-card {"
        "  background-color: rgba(255, 255, 255, 0.98);"
        "  color: #2c3e50;"
        "  border: 1px solid rgba(0,0,0,0.12);"
        "  border-radius: 12px;"
        "  padding: 16px;"
        "  box-shadow: 0 5px 18px rgba(0,0,0,0.08);"
        "}"

        "flowboxchild:selected, "
        "flowboxchild:selected * {"
        "  color: #2c3e50 !important;" 
        "}"
        "flowboxchild:selected {"
        "  background-color: transparent !important;" 
        "}"

        "#main-window .student-card + .student-card {"
        "  margin-top: 12px;"
        "}"

        "#main-window .student-card hr {"
        "  margin-top: 12px;"
        "  margin-bottom: 12px;"
        "  border: none;"
        "  border-top: 1px solid rgba(0,0,0,0.08);"
        "}"

        ".transparent, .transparent viewport {"
        "  background-color: transparent;"
        "  background-image: none;"
        "}"
        
        /* Make back-button always have visible white background */
        ".back-button {"
        "  background-image: none;"
        "  background-color: #ffffff;"
        "  color: #000000;"
        "  border-radius: 6px;"
        "  border: 1px solid rgba(0,0,0,0.08);"
        "}"

        ".back-button:hover {"
        "  background-color: #f6f6f6;"
        "}"
        ".label-btn label {"
        "   font-size: 20px;"
        "   font-weight: bold;"
        "}"

        ".border-label {"
        "   border: 2px solid black;"
        "   border-radius: 6px;"
        "   padding: 6px;"
        "}"
        
        ".readonly-entry {"
        "   background-color: #f0f0f0;"
        "   color: #666;"
        "}"
        
        "* {"
        "   font-size:16px;"
        "}"
        
        ".preview-text {"
        "   font-family: Consolas;"
        "   font-size: 14px;"
        "}"
    );

    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    // ==========================================

    load_last_filename();

    SinhvienWidgets *widgets = g_malloc0(sizeof(SinhvienWidgets));

    widgets->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(widgets->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);

    gtk_stack_add_named(GTK_STACK(widgets->stack), create_file_selection_ui(widgets), "file_select");
    gtk_stack_add_named(GTK_STACK(widgets->stack), create_menu_ui(widgets), "menu");
    gtk_stack_add_named(GTK_STACK(widgets->stack), create_add_sv_ui(widgets), "add");
    gtk_stack_add_named(GTK_STACK(widgets->stack), create_list_sv_ui(widgets), "list");
    // gtk_stack_add_named(GTK_STACK(widgets->stack), create_search_sv_ui(widgets), "search");
    gtk_stack_add_named(GTK_STACK(widgets->stack), create_delete_sv_ui(widgets), "delete");
    gtk_stack_add_named(GTK_STACK(widgets->stack), create_sort_sv_ui(widgets), "sort");
    gtk_stack_add_named(GTK_STACK(widgets->stack), create_cap_mssv_ui(widgets), "cap_mssv");
    gtk_stack_add_named(GTK_STACK(widgets->stack), create_cap_email_ui(widgets), "cap_email");
    gtk_stack_add_named(GTK_STACK(widgets->stack), create_preview_ui(widgets), "preview");

    GtkWidget *scroll = gtk_scrolled_window_new();

    // Áp dụng class transparent cho cả scroll (sẽ truyền xuống viewport) và stack
    gtk_widget_add_css_class(scroll, "transparent");
    gtk_widget_add_css_class(widgets->stack, "transparent");

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
        GTK_POLICY_NEVER,       // Không scroll ngang
        GTK_POLICY_AUTOMATIC);  // Scroll dọc tự động khi nội dung vượt chiều cao window
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroll), widgets->stack);
    gtk_window_set_child(GTK_WINDOW(window), scroll);

    gtk_window_present(GTK_WINDOW(window));
}
int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.pbl1.quanly", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
