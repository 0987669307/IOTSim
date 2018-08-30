            Hướng dẫn chạy các file
Các file được để ở thư mục ~/ns-allinone-3.28/ns-3.28/scratch
Để chạy file thì vào thư mục ~/ns-allinone-3.28/ns-3.28, mở terminal và gõ lệnh như bên dưới
1) File colectdata.cc và energy-task.cc:
- colectdata.cc: thu thập dữ liệu nhiệt độ để xác định vùng cháy, tính số UAV cần thiết và tọa độ mà các UAV sẽ đi tới
  Chạy file: ./waf --run scratch/colectdata
- energy-task.cc: các UAV đi tới vị trí tìm được ở file colectdata.cc để xử lý, đồng thời tính năng lượng mà các UAV tiêu tốn.
  
  Chạy file: ./waf --run scratch/energy-task
             gnuplot Energy-2d.plt
  Sau khi chạy xong thì đồ thị năng lượng có thể xem ở file Energy-2d.png tại thư mục ~/ns-allinone-3.28/ns-3.28
2) File establishconnection.cc, subcribe.cc và MQTT.cc: mô phỏng truyền theo giao thức MQTT
- establishconnection.cc: thiết lập kết nối giữa client và broker
  Chạy file: ./waf --run/scratch/establishconnection
- subcribe.cc: client subcribe topic
  Chạy file: ./waf --run scratch/subcribe
- MQTT.cc: gửi tin theo giao thức MQTT và tính tỉ lệ truyền thành công
  Chạy file: ./waf --run scratch/MQTT              
3) task.cc: 2 UAV xử lý tác vụ theo mức độ ưu tiên định trước, có phát sinh tác vụ mới, có giúp đỡ nhau xử lý sự kiện
Sau khi chạy file này, ta sẽ sử dụng NetAnim để trực quan hóa quá trình mô phỏng
Chạy file: ./waf --run scratch/task
Sau khi chạy lệnh này, file task.xml sẽ được tạo ra tại thư mục ~/ns-allinone-3.28/ns-3.28, file này được dùng trong NetAnim
Hướng dẫn cài đặt NetAnim các phiên bản tại link: https://www.nsnam.org/wiki/NetAnim
Sau khi cài đặt xong, taị thư mục ~/ns-allinone-3.28/netanim-3.108, mở terminal và gõ lệnh ./NetAnim để mở phần mềm NetAnim.
Load file task.xml ở trên và bấm bắt đầu để chạy mô phỏng

