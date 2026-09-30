# FileDatePrefixer

[English](README.md) | [Türkçe](README-tr.md)

**FileDatePrefixer is a small Windows utility that adds, updates, or removes a sortable date prefix in file and folder names.**

The prefix format is:

```text
YYYYMMDDHHMM <original name>
```

For example:

```text
report.pdf
        │
        ▼
202501052345 report.pdf
```

The timestamp can come either from the **current local system time** or from the selected file/folder's **last modification time**.

## Features

- Works with both files and directories.
- Supports **batch processing from Windows Explorer**: select multiple files/folders and invoke a context-menu command to prefix, update, or remove dates across the selection. Windows Explorer invokes the shell command for the selected items; each FileDatePrefixer process handles one path.
- Adds a `YYYYMMDDHHMM` prefix using the current local system time.
- Can instead use the selected item's last modification time.
- Detects an existing prefix and **replaces it** instead of stacking another date.
- Can remove an existing date prefix.
- Uses `std::filesystem::rename()`; file contents are not rewritten.
- Native Windows application with no runtime UI during a successful operation.
- Includes Win32 and x64 Visual Studio build configurations.
- MIT licensed.

## Usage Demo

### Direct / Drag-and-Drop Use

![Direct use demonstration](Contents/Direct.gif)

Dropping a file or folder onto the executable invokes it with that path as the first command-line argument. With no second option, the application uses the current system time.

### Explorer Context-Menu Use

![Windows Explorer context-menu demonstration](Contents/Context.gif)

The project was designed to be callable from Windows Explorer shell commands such as **Prefix Date** and **Prefix Date (File/Folder Time)**.

## Command-Line Interface

```text
FileDatePrefixer.exe <path> [option]
```

### Use Current System Time

```text
FileDatePrefixer.exe "C:\path\report.pdf"
```

or explicitly:

```text
FileDatePrefixer.exe "C:\path\report.pdf" --use-system-time
```

Result:

```text
report.pdf
→ 202501052345 report.pdf
```

If the name already begins with a valid 12-digit prefix followed by a space, that prefix is replaced:

```text
202412010830 report.pdf
→ 202501052345 report.pdf
```

### Use File/Folder Modification Time

```text
FileDatePrefixer.exe "C:\path\report.pdf" --use-file-time
```

The prefix is generated from `std::filesystem::last_write_time()` after conversion to the local system-clock representation.

### Remove the Date Prefix

```text
FileDatePrefixer.exe "C:\path\202501052345 report.pdf" --remove-date
```

Result:

```text
202501052345 report.pdf
→ report.pdf
```

If the name does not begin with the expected date-prefix pattern, the remove operation leaves the filename text unchanged and the program still attempts the rename.

## Prefix Recognition

The implementation recognizes an existing prefix using:

```regex
^\d{8}\d{4}\s
```

In practical terms, the filename must begin with:

- exactly **12 digits**;
- followed by one whitespace character.

The code treats those 12 digits structurally as `YYYYMMDDHHMM`, but the regular expression itself does not validate whether the month, day, hour, or minute values form a real date.

Examples:

```text
202501052345 report.pdf    recognized
202501052345_report.pdf    not recognized
20250105 report.pdf        not recognized
report 202501052345.pdf    not recognized
```

## Runtime Flow

```text
Windows command line
        │
        ▼
CommandLineToArgvW()
        │
        ├── no path ─────────────► error MessageBox
        │
        ▼
validate path exists
        │
        ├── missing ─────────────► error MessageBox
        │
        ▼
inspect option
        │
        ├── --remove-date
        │       │
        │       └──► strip matching prefix
        │
        ├── --use-file-time
        │       │
        │       └──► read last_write_time()
        │
        └── default / --use-system-time
                │
                └──► read local system time
                         │
                         ▼
                build destination name
                         │
                         ▼
                std::filesystem::rename()
```

Filesystem exceptions are caught and displayed through a Windows message box.

## Date Generation

### Current Time

`getCurrentDate()` uses:

- `std::chrono::system_clock::now()`;
- `std::chrono::system_clock::to_time_t()`;
- `localtime_s()`.

The result is formatted without separators as:

```text
YYYY MM DD HH MM
 │   │  │  │  └─ minute
 │   │  │  └──── hour
 │   │  └─────── day
 │   └────────── month
 └────────────── year
```

### File Modification Time

`getFileModificationDate()` reads `std::filesystem::last_write_time()`, converts the filesystem clock value to a `system_clock` time point, then formats it using the same local-time representation.

## Rename Behavior

The utility operates only on the selected path's filename. The parent directory remains unchanged.

```text
C:\Projects\report.pdf
          │
          ▼
C:\Projects\202501052345 report.pdf
```

The actual filesystem operation is:

```cpp
fs::rename(filePath, newFilePath);
```

This means the program performs a rename/move operation inside the same parent path rather than copying file contents and deleting the original.

Normal filesystem restrictions still apply. A rename can fail because of permissions, an invalid destination, a conflicting filename, a locked resource, or other filesystem conditions.

## Error Handling

The executable reports errors with Windows message boxes.

Handled cases include:

| Condition | Behavior |
|---|---|
| No target path supplied | Error message, exit code 1 |
| Target path does not exist | Error message, exit code 1 |
| `std::filesystem` operation throws | Exception text shown, exit code 1 |
| Successful operation | Exit code 0 |

The source does not contain a success dialog.

## Build

The repository contains a Visual Studio solution:

```text
FileDatePrefixer.sln
```

and a Visual C++ project:

```text
FileDatePrefixer/FileDatePrefixer.vcxproj
```

### Toolchain

The current project file specifies:

- Visual Studio C++ project format version 17;
- MSVC platform toolset `v143`;
- Windows SDK target `10.0`;
- C++17 explicitly for x64 configurations;
- Unicode character set;
- Debug/Release configurations for Win32 and x64.

The **Release x64** configuration uses the Windows subsystem, so the normal release executable runs without opening a console window.

### Build with Visual Studio

1. Open `FileDatePrefixer.sln`.
2. Select the desired configuration, typically `Release | x64`.
3. Build the `FileDatePrefixer` project.
4. Use the generated `FileDatePrefixer.exe` directly or integrate it into an Explorer shell command.

## Releases

The repository has packaged releases. The latest release currently published is **v1.0.4**, whose release notes identify date-prefix removal as the added feature.

[Download the latest release](https://github.com/sezgynus/file-date-prefixer/releases/latest)

The release package includes the deployment scripts used to install and remove the Windows Explorer context-menu integration.

## Windows Explorer Integration

The executable is suitable for Windows shell verbs because it accepts the selected item path as its first argument and performs the operation without requiring an interactive UI.

Typical shell commands map naturally to:

```text
Prefix Date
    FileDatePrefixer.exe "%1" --use-system-time

Prefix Date (File/Folder Time)
    FileDatePrefixer.exe "%1" --use-file-time

Remove Date Prefix
    FileDatePrefixer.exe "%1" --remove-date
```

The packaged release provides the installation/removal scripts for registering these commands with Windows Explorer.

## Implementation Notes

The application entry point is `WinMain`, while command-line parsing uses `GetCommandLineW()` and `CommandLineToArgvW()` so Windows paths are obtained as wide strings.

The selected path is stored as `std::filesystem::path`. The filename-processing helpers currently convert `path.filename()` to `std::string` before applying `std::regex`. Users working with filenames outside the active Windows narrow-character encoding should therefore test those names carefully; the implementation is not a fully wide-character filename-processing path end to end.

The code accepts an arbitrary second argument as the time option. Only `--use-system-time` and `--use-file-time` assign a timestamp. An unsupported option is not explicitly rejected before the rename path is built, so callers should use only the documented options.

## Source Map

| File | Responsibility |
|---|---|
| `FileDatePrefixer/FileDatePrefixer.cpp` | Command-line parsing, date generation, prefix recognition/removal and filesystem rename |
| `FileDatePrefixer/FileDatePrefixer.vcxproj` | Visual Studio build configurations and toolchain settings |
| `FileDatePrefixer/Resources.rc` | Windows resource script |
| `FileDatePrefixer/icon.ico` | Application icon |
| `Contents/Direct.gif` | Direct/drag-and-drop usage demonstration |
| `Contents/Context.gif` | Explorer context-menu usage demonstration |
| `LICENSE` | MIT license |

## Repository Structure

```text
file-date-prefixer/
├── .github/
│   └── workflows/
│       └── sign-commits.yml
├── Contents/
│   ├── Context.gif
│   └── Direct.gif
├── FileDatePrefixer/
│   ├── FileDatePrefixer.cpp
│   ├── FileDatePrefixer.vcxproj
│   ├── FileDatePrefixer.vcxproj.filters
│   ├── Resources.rc
│   ├── icon.ico
│   └── resources.h
├── FileDatePrefixer.sln
├── LICENSE
├── README.md
└── README-tr.md
```

## License

FileDatePrefixer is distributed under the [MIT License](LICENSE).

Copyright © 2025 Sezgin AÇIKGÖZ.
