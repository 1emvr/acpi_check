#include <windows.h>
#include <sysinfoapi.h>
#include <cstdint>
#include <cstdio>

int main() {
     DWORD acpi = 'ACPI';
     UINT size = EnumSystemFirmwareTables(acpi, nullptr, 0);

     if (!size) {
          printf("Failed to get system firmware tables: 0x%lx\n", GetLastError());
          return 1;
     }

     void *buffer = HeapAlloc(GetProcessHeap(), 0, size);
     if (!buffer) {
          printf("Failed to allocate a buffer");
          return 1;
     }

     UINT read = EnumSystemFirmwareTables(acpi, buffer, size);

     SIZE_T table_count = size/4;
     CHAR **table_names = (char**)HeapAlloc(GetProcessHeap(), 0, table_count * sizeof(char*));

     for (size_t i = 0; i < table_count; i++) {
          table_names[i] = (char*)HeapAlloc(GetProcessHeap(), 0, 5);
          memcpy(table_names[i], (char*)buffer + (i * 4), 4);
          table_names[i][4] = '\0';
     }

     printf("ACPI Tables:\n");
     for (size_t i = 0; i < table_count; i++) {
          printf("\t%s\n", table_names[i]);
     }

     if (buffer) {
          HeapFree(GetProcessHeap(), 0, buffer);
     }
     if (table_names) {
          for (size_t i = 0; i < table_count; i++) {
               if (table_names[i]) {
                    HeapFree(GetProcessHeap(), 0, table_names[i]);
               }
          }
          HeapFree(GetProcessHeap(), 0, table_names);
     }
     return 0;
}
