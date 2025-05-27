NTFS Master File Table (MFT) Concepts

The Master File Table (MFT) is a critical component of the NTFS file system, serving as a database that stores metadata about every file and directory. Each MFT entry contains various attributes that describe the file's properties and data. Below are key concepts related to MFT attributes and structures.

Attributes

Attributes are the building blocks of an MFT entry, storing metadata and data for a file or directory. Each attribute has a type (e.g., file name, data) and can be resident (stored within the MFT entry) or non-resident (stored externally with a pointer). Attributes define the file's properties, location, and content.

Bitmap

The $Bitmap attribute tracks the allocation status of clusters in the volume or MFT. For the MFT, it indicates which MFT entries are in use. Each bit represents an MFT entry or cluster, with 1 indicating allocated and 0 indicating free, enabling efficient space management.

Data

The $DATA attribute contains the actual content of a file. It can be resident (small data stored directly in the MFT) or non-resident (larger data stored in clusters with pointers in the MFT). Alternate data streams (ADS) allow multiple data attributes for a single file.

File Name

The $FILE_NAME attribute stores the file or directory name, including short (8.3 format) and long names. It includes metadata like creation, modification, and access times. Multiple $FILE_NAME attributes may exist for compatibility (e.g., DOS names) or hard links.

Index

Index attributes, such as $INDEX_ROOT and $INDEX_ALLOCATION, manage directory contents. They store file names and references in a B-tree structure for efficient lookup and sorting. $INDEX_ROOT holds small indexes, while $INDEX_ALLOCATION points to external clusters for larger directories.

Object ID

The $OBJECT_ID attribute assigns a unique identifier (GUID) to a file or directory, enabling tracking across moves or renames. It supports features like distributed link tracking and is optional, not present in all MFT entries.

Reparse Point

The $REPARSE_POINT attribute marks a file or directory with special behavior, such as symbolic links, junctions, or deduplicated files. It contains data that defines the reparse behavior, redirecting access to another location or process.

Security Descriptor

The $SECURITY_DESCRIPTOR attribute stores access control information, including permissions, ownership, and auditing rules. It defines who can access the file and what actions they can perform, often referencing a shared descriptor in the $Secure system file.

Standard Information

The $STANDARD_INFORMATION attribute contains core metadata, such as creation, modification, and access timestamps, file attributes (e.g., read-only, hidden), and ownership details. It is always resident and provides essential file properties.

Volume

The $VOLUME_INFORMATION and $VOLUME_NAME attributes (in the $Volume system file) store volume-level metadata, such as the NTFS version, volume label, and flags (e.g., dirty status). They are not part of individual file MFT entries but are critical for the file system.

xattr

Extended attributes (xattr) are not a standard NTFS attribute but are supported in some contexts (e.g., Windows Subsystem for Linux). They store additional metadata key-value pairs for compatibility with non-NTFS systems, typically managed outside the core MFT structure.
