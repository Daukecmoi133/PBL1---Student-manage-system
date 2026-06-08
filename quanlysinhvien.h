#ifndef QUANLYSINHVIEN_H
#define QUANLYSINHVIEN_H

#include <stddef.h>

// source ICON
#define MENU_ICON "C:/Users/admin/OneDrive/Desktop/PBL1/icon/student_manager_icon.png"
#define ADD_ICON "C:/Users/admin/OneDrive/Desktop/PBL1/icon/student_add_icon.png"
#define LIST_ICON "C:/Users/admin/OneDrive/Desktop/PBL1/icon/student_list_icon.png"
#define SEARCH_ICON "C:/Users/admin/OneDrive/Desktop/PBL1/icon/student_search_icon.png"
#define REMOVE_ICON "C:/Users/admin/OneDrive/Desktop/PBL1/icon/student_repair_icon.png"
#define SORT_ICON "C:/Users/admin/OneDrive/Desktop/PBL1/icon/student_sort_icon.png"
#define MSSV_ICON "C:/Users/admin/OneDrive/Desktop/PBL1/icon/student_mssv_icon.png"
#define EMAIL_ICON "C:/Users/admin/OneDrive/Desktop/PBL1/icon/student_email_icon.png"
#define BK_PIC "C:/Users/admin/OneDrive/Desktop/PBL1/icon/DUT_BK_Logo.png"
#define ITF_PIC "C:/Users/admin/OneDrive/Desktop/PBL1/icon/DUT_ITF_Logo.png"

//

struct NgaySinh
{
    int day, month, year;
};

struct SinhVien
{
    long long MaSV;
    int MaKhoa;
    char Name[50], Gender[10], Address[150], Class[10], Email[50];
    struct NgaySinh NgaySinh;
};

void setColor(int color); // điều chỉnh màu
void clearScreen(); // dọn sạch termintal
void loading1s(); // loading frame
void vietHoaTatCa(char *s); // upper toàn bộ string s
void vietHoaChuDau(char *s); // upper s[0] và lower phần còn lại
void xoaBoNhoDem(); // clear bộ nhớ
int nhapSoNguyen(); // check input số nguyên
void TachLop(char* s, char* sInt, char* sStr); // tách tên lớp = khóa + tên lớp
int layMaKhoaTuLop(const char *className); // lấy mã khoa từ tên lớp
int class_name_has_section(const char *className); // kiểm tra xem tên lớp có phần mã lớp hợp lệ hay không
void get_class_group_prefix(const char *className, char *prefix_out); // lấy tiền tố nhóm lớp từ tên lớp (ví dụ: "21" từ "21T_DT1")
int count_students_in_class(const struct SinhVien ArrSinhVien[], int slsv, const char *className); // đếm số sinh viên thuộc lớp nhất định
void build_class_limit_message(const struct SinhVien ArrSinhVien[], int slsv, const char *className, char *out_msg, size_t out_len); // tạo thông báo giới hạn số lượng hoặc thông tin lớp
int is_valid_export_filename(const char *filename); // kiểm tra tên file xuất có hợp lệ không
int export_student_list_to_file(const char *filename, const char *classFilter, const struct SinhVien ArrSinhVien[], int slsv); // xuất danh sách sinh viên ra file, có thể lọc theo lớp
int LuuVaoFile(struct SinhVien ArrSinhVien[], int slsv); // lưu thông tin vào file
int NhapTuFile(struct SinhVien ArrSinhVien[], int *slsv, const char *fileName); //lấy thôngg tin từ file
void XuatDanhSachSinhVien(const struct SinhVien ArrSinhVien[], int slsv); // xuất danh sách sinh viên ra file từ menu console
void SuaSinhVien(struct SinhVien ArrSinhVien[], int slsv); // sửa thông tin sinh viên từ menu console
void load_last_filename();
void save_last_filename();
const char* layTen(const char* hoTen); // lấy tên trong string họ và tên
int checkDate(int day, int month, int year); // check input ngày sinh
int checkClass(char* s); // check input Khóa
int SoSanhTenViet(const char *name1, const char *name2); // so sánh tên theo tiếng Việt
void CapMaSoSinhVien(struct SinhVien ArrSinhVien[], int slsv); // cấp mã số sinh viên
void CapEmailSinhVien(struct SinhVien ArrSinhVien[], int slsv); // cấp email sinh viên
void SapXepTheoLop(struct SinhVien ArrSinhVien[], int slsv); // sắp xếp theo lớp
void SapXepTheoNgaySinh(struct SinhVien arr[], int count); // sắp xếp theo ngày tháng năm sinh
void SapXepTheoMSSV(struct SinhVien arr[], int count); // sắp xếp theo MSSV (chỉ với những người đã có MSSV)
void SapXepTheoTen(struct SinhVien ArrSinhVien[], int slsv); // sắp xếp theo tên và cập nhật DaSapXep
void sort_students_by_name(struct SinhVien arr[], int count);
void sort_students_smart(struct SinhVien arr[], int count);
void ThemSinhVien(struct SinhVien ArrSinhVien[], int *slsv); //lấy input infor sinh viên
void TimKiemSinhVien(struct SinhVien ArrSinhVien[], int slsv); // search sinh viên
void XoaSinhVien(struct SinhVien ArrSinhVien[], int *slsv); // remove sinh viên
void InDanhSachSinhVien(struct SinhVien ArrSinhVien[], int slsv); // in toàn bộ danh sách sinh viên
extern char CurrentFileName[1000];

#endif // QUANLYSINHVIEN_H