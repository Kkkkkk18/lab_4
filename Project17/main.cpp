
#include <cmath>
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <ctime>


#pragma comment (lib,"gdi32.lib")
#pragma comment (lib,"user32.lib")

const int kMinimalLength = 30;
const int kMinimalLength_2 = kMinimalLength*kMinimalLength;
const float eps = 0.001f;

// Главная функция программы
int main(int argc, char ** argv) {
  if (argc < 3) {
    std::cout << "Usage: lab.exe <graph.in> <graph.bmp>\n";
    return 1;
  }

  std::ifstream in(argv[1]);
  if (in) {
    int V, E; // Число вершин и ребер

    in >> V >> E;

    std::vector<std::vector<int>> edges(V); // Ребра
    std::vector<float> vertice_x(V); // Горизонтальные координаты вершин
    std::vector<float> vertice_y(V); // Вертикальные координаты вершин
    std::vector<int> pows(V); // Степени вершин
    std::vector<std::vector<float>> distance_x(V); // Расстояния по x
    std::vector<std::vector<float>> distance_y(V); // Расстояния по y
    std::vector<std::vector<float>> r2(V); // Квадраты расстояний
    std::vector<float> force_x(V); // Горизонтальные силы
    std::vector<float> force_y(V); // Вертикальные силы
    std::vector<float> old_force_x(V); // Предыдущие горизонтальные силы
    std::vector<float> old_force_y(V); // Предыдущие вертикальные силы

    for (int i = 0; i < E; i++) { // Цикл чтения ребер
      int u, v; // Номера вершин
      in >> u >> v;
      edges[u].push_back(v);
      pows[u]++;
      pows[v]++;
    }
    in.close();

    int max_pow = 0;
    for (int i = 0; i < V; i++)
      if (pows[i] > max_pow)
        max_pow = pows[i];

    std::srand(std::time(0));
    for (int i = 0; i < V; i++) { // Цикл начального заполнения координат
      float angle = 2.0*3.14159265358*std::rand()/RAND_MAX;
      vertice_x[i] = (5.0 + V*(1.0-1.0*pows[i]/max_pow)*cos(angle));
      vertice_y[i] = (5.0 + V*(1.0-1.0*pows[i]/max_pow)*sin(angle));
      distance_x[i].resize(V);
      distance_y[i].resize(V);
      r2[i].resize(V);
    }

    float max_fx = 0.0f;
    float max_fy = 0.0f;
    bool oscillating = false;
    int iterations = (int)(200*sqrt(V));
    do { // Цикл размещения вершин
      // Вычисление расстояний
      for (int i = 0; i < V; i++) {
        force_x[i] = 0.0f;
        force_y[i] = 0.0f;
        for (int j = 0; j < i; j++) {
          float dx = vertice_x[j] - vertice_x[i];
          float dy = vertice_y[j] - vertice_y[i];
          distance_x[i][j] = dx;
          distance_x[j][i] = -dx;
          distance_y[i][j] = dy;
          distance_y[j][i] = -dy;
          r2[i][j] = r2[j][i] = dx*dx + dy*dy;
        }
      }
      // Вычисляем влияние ребер
      for (int i = 0; i < V; i++) {
        int adjacents = edges[i].size();
        for (int j = 0; j < adjacents; j++) {
          int v = edges[i][j];
          float dx, dy;
          float r_2 = r2[i][v];
          if (r_2 < kMinimalLength_2) {
            dx = -5.0 * kMinimalLength_2*kMinimalLength_2*kMinimalLength /
                       ((r_2 + 1)*(r_2 + 1)*(r_2 + 1)) * distance_x[i][v];
            dy = -5.0 * kMinimalLength_2*kMinimalLength_2*kMinimalLength /
                       ((r_2 + 1) * (r_2 + 1)*(r_2 + 1)) * distance_y[i][v];
          } else {
            dx = 1.0 * r_2 / kMinimalLength_2 / kMinimalLength *
                 distance_x[i][v];
            dy = 1.0 * r_2 / kMinimalLength_2 / kMinimalLength *
                 distance_y[i][v];
          }
          force_x[i] += dx;
          force_x[v] -= dx;
          force_y[i] += dy;
          force_y[v] -= dy;
        }
      }
      // Вычисление отталкиваний вершин
      for (int i = 0; i < V; i++) {
        for (int j = 0; j < i; j++) {
          float dx, dy;
          float r_2 = r2[i][j];
          dx = -500.0 * kMinimalLength_2 * kMinimalLength /
                     (r_2 + 1) / (r_2 + 1) * distance_x[i][j];
          dy = -500.0 * kMinimalLength_2 * kMinimalLength /
                     (r_2 + 1) / (r_2 + 1) * distance_y[i][j];
          force_x[i] += dx;
          force_x[j] -= dx;
          force_y[i] += dy;
          force_y[j] -= dy;
        }
      }
      max_fx = 0.0f;
      max_fy = 0.0f;
      // Делаем шаг
      for (int i = 0; i < V; i++) {
        float fx = force_x[i];
        float fy = force_y[i];
        if (fabs(fx) > max_fx)
           max_fx = fabs(fx);
        if (fabs(fy) > max_fy)
           max_fy = fabs(fy);
      }
      oscillating = true;
      float tau = 10 / (max_fx + max_fy + 1.0f);
      for (int i = 0; i < V; i++) {
        float fx = force_x[i];
        float fy = force_y[i];
        vertice_x[i] += 1.0*tau*force_x[i];
        vertice_y[i] += 1.0*tau*force_y[i];
        if (fx*old_force_x[i] >= 0.0 || fy*old_force_y[i] >= 0.0)
          oscillating = false;
        old_force_x[i] = force_x[i];
        old_force_y[i] = force_y[i];
      }
      if ((iterations % 10) == 0)
        std::cout << "Iterations rest: " << iterations << std::endl;
    } while ((--iterations) && !oscillating && max_fx + max_fy > eps);
    // Отрисовка
    float min_x = 1E10;
    float max_x = -1E10;
    float min_y = 1E10;
    float max_y = -1E10;
    for (int i = 0; i < V; i++) { // Находим границы изображения
      float x = vertice_x[i];
      float y = vertice_y[i];
      if (x < min_x)
         min_x = x;
      if (x > max_x)
         max_x = x;
      if (y < min_y)
         min_y = y;
      if (y > max_y)
         max_y = y;
    }
    min_x -= 30;
    max_x += 30;
    min_y -= 30;
    max_y += 30;
    int size_x = (int)(max_x - min_x) + 1;
    int size_y = (int)(max_y - min_y) + 1;

    LONG image_size = size_y * ((size_x * 3 + 3) & 0xFFFFFFFC);

    char * bits = new char[image_size]; // Выделяем память под изображение
    for (int p = 0; p < image_size; p++)
        bits[p] = 0;

    HDC win_dc = GetDC(GetConsoleWindow());
    HDC hdc = CreateCompatibleDC(win_dc);
    HBITMAP bmp = CreateCompatibleBitmap(win_dc, size_x, size_y);

    SelectObject(hdc, bmp);

    HPEN hpen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
    SelectObject(hdc, hpen);

    HFONT hfont = CreateFont (
      -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72),
      0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, 
      OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
      DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));
    SelectObject(hdc, hfont);
    // Рисуем вершины
    for (int i = 0; i < V; i++) {
      char buf[20];
      int cx = (int)(vertice_x[i] - min_x);
      int cy = (int)(vertice_y[i] - min_y);
      Ellipse(hdc, cx - 4, cy - 4, cx + 4, cy + 4);
      sprintf_s(buf, "%i", i);
      TextOutA(hdc, cx + 4, cy + 4, buf, strlen(buf));
    }
    // Рисуем ребра
    for (int i = 0; i < V; i++) {
      int adjacents = edges[i].size();
      for (int j = 0; j < adjacents; j++) {
        int v = edges[i][j];
        MoveToEx(hdc, (int)(vertice_x[i] - min_x), (int)(vertice_y[i] - min_y),
                 NULL);
        LineTo(hdc, (int)(vertice_x[v] - min_x), (int)(vertice_y[v] - min_y));
      }
    }
    // Заполняем структуры для сохранения BMP-файла
    BITMAPINFOHEADER BMIH;

    BMIH.biSize = sizeof(BITMAPINFOHEADER);
    BMIH.biSizeImage = size_x * size_y * 3;
    BMIH.biWidth = size_x;
    BMIH.biHeight = size_y;
    BMIH.biPlanes = 1;
    BMIH.biBitCount = 24;
    BMIH.biCompression = BI_RGB;
    BMIH.biSizeImage = size_x * size_y * 3;

    BITMAPFILEHEADER bmfh;

    int n_bits_offset = sizeof(BITMAPFILEHEADER) + BMIH.biSize;
    LONG file_size = n_bits_offset + image_size;

    bmfh.bfType = 'B'+('M'<<8);
    bmfh.bfOffBits = n_bits_offset;
    bmfh.bfSize = file_size;
    bmfh.bfReserved1 = bmfh.bfReserved2 = 0;

    std::ofstream out(argv[2], std::ios::binary | std::ios::out |std::ios::trunc);
    if (out) { // Сохраняем bmp-файл
      out.write((const char *)&bmfh, sizeof(BITMAPFILEHEADER));
      out.write((const char *)&BMIH, sizeof(BITMAPINFOHEADER));

      GetDIBits(hdc, bmp, 0, size_y, bits, (BITMAPINFO*)&BMIH, DIB_RGB_COLORS);
      out.write(bits, image_size);

      out.close();
    }

    delete[] bits;

    DeleteDC(hdc);
    DeleteObject(hfont);
    DeleteObject(hpen);
    DeleteObject(bmp);
  } else {
    std::cout << "Can't open file '" << argv[1] << "'\n";
    return 2;
  }

  return 0;
}