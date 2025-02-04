# Hệ thống Điều khiển RPM với Phản hồi

Dự án này được thiết kế để điều khiển tốc độ vòng quay (RPM) của động cơ bằng cách sử dụng vòng phản hồi với đầu ra PWM và một công tắc DIP để cài đặt RPM mong muốn. Hệ thống đo lường RPM của động cơ và điều chỉnh chu kỳ nhiệm vụ PWM để đạt được RPM mong muốn. RPM hiện tại được hiển thị trên một màn hình 7-segment 3 chữ số.

### Tính năng:
- **Đầu vào DIP Switch**: RPM mong muốn của động cơ được chọn bằng cách sử dụng công tắc DIP 3-bit, cho phép 8 cài đặt RPM khác nhau.
- **Điều khiển PWM**: Đầu ra PWM điều chỉnh tốc độ động cơ dựa trên sự khác biệt giữa RPM đo được và RPM mong muốn.
- **Đo RPM**: Hệ thống đếm các cạnh rơi của tín hiệu để tính toán RPM.
- **Điều khiển Phản hồi**: Hệ thống điều chỉnh đầu ra PWM để giảm thiểu sai số giữa RPM hiện tại và RPM mong muốn.
- **Hiển thị RPM**: RPM hiện tại được hiển thị trên màn hình 7-segment 3 chữ số.

### Các thành phần sử dụng:
- **Vi điều khiển**: 8051 (REG51.H)
- **Điều khiển PWM**: Sử dụng để điều chỉnh tốc độ động cơ.
- **Công tắc DIP**: Dùng để chọn cài đặt RPM.
- **Màn hình 7-segment**: Dùng để hiển thị RPM hiện tại.
- **Ngắt**: Dùng để đo RPM và cập nhật điều khiển phản hồi mỗi 50ms.
