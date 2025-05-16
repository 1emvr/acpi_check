// Credits to https://revers.engineering/evading-trivial-acpi-checks/
#include <cstdint>
struct acpi_table_header_format {
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[ 6 ];
    uint64_t oem_table_id;
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
};

std::unordered_set<std::string> disallowed_acpi_entries = {{"WAET"}, [...]};
std::unordered_set<std::string> disallowed_oem_ids = {
    {"VMWARE"}, {"VBOX"}, {"BOCHS"}, {"VRTUAL"}, {"PRLS"},
};

void check_firmware_tables() {
	constexpr unsigned long acpi_signature = 'ACPI';

	buffer<uint32_t> buf;
	buf.resize(0x1000);

	enumerate_fw_tables(acpi_signature, buf.get(), 0x1000);
	std::vector<std::pair<std::string, uint32_t>> firmware_table_ids{};

	for (auto it = 0; buf.get()[it] != 0; it++) {
		char tid[6] = {0};

		memcpy(tid, &buf.get()[it], sizeof(uint32_t));
		firmware_table_ids.emplace_back(std::make_pair(tid, buf.get()[it]));

		if (disallowed_acpi_entries.contains(tid)) {
			elog::critical("acpi entry indicates virtual environment (%s).", tid);
		}
	}

	for (const auto &table_id : firmware_table_ids | std::views::values) {
		buffer<uint8_t> fwt_buffer;
		fwt_buffer.resize(0x100);

		if (const auto ret = get_fw_table_info(acpi_signature, table_id, fwt_buffer.get(), fwt_buffer.size()); ret == 0) {
			continue;
		}
		auto *hdr = reinterpret_cast<acpi_table_header_format *>(fwt_buffer.get());
		char oem_id[8] = {0};
		memcpy(oem_id, hdr->oem_id, 0x6);

		if (disallowed_oem_ids.contains(oem_id)) {
			elog::critical("acpi oem id indicates virtual environment (%s).", oem_id);
		}
	}
}

