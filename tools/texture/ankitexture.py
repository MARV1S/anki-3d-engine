#!/usr/bin/python

import optparse
import subprocess
import re
import os
import struct
import copy
import tempfile
import shutil

#
# Config
#

class Config:
	in_files = []
	out_file = ""
	fast = False
	type = 0 
	normal = False
	convert_path = ""
	no_alpha = False
	no_uncompressed = False
	to_linear_rgb = False

	tmp_dir = ""

#
# AnKi texture
#

# Texture type
TT_NONE = 0
TT_2D = 1
TT_CUBE = 2
TT_3D = 3
TT_2D_ARRAY = 4

# Color format
CF_NONE = 0
CF_RGB8 = 1
CF_RGBA8 = 2

# Data compression
DC_NONE = 0
DC_RAW = 1 << 0
DC_ETC2 = 1 << 1
DC_S3TC = 1 << 2

#
# DDS
#

# dwFlags of DDSURFACEDESC2
DDSD_CAPS           = 0x00000001
DDSD_HEIGHT         = 0x00000002
DDSD_WIDTH          = 0x00000004
DDSD_PITCH          = 0x00000008
DDSD_PIXELFORMAT    = 0x00001000
DDSD_MIPMAPCOUNT    = 0x00020000
DDSD_LINEARSIZE     = 0x00080000
DDSD_DEPTH          = 0x00800000

# ddpfPixelFormat of DDSURFACEDESC2
DDPF_ALPHAPIXELS    = 0x00000001
DDPF_FOURCC         = 0x00000004
DDPF_RGB            = 0x00000040

# dwCaps1 of DDSCAPS2
DDSCAPS_COMPLEX     = 0x00000008
DDSCAPS_TEXTURE     = 0x00001000
DDSCAPS_MIPMAP      = 0x00400000

# dwCaps2 of DDSCAPS2
DDSCAPS2_CUBEMAP            = 0x00000200
DDSCAPS2_CUBEMAP_POSITIVEX  = 0x00000400
DDSCAPS2_CUBEMAP_NEGATIVEX  = 0x00000800
DDSCAPS2_CUBEMAP_POSITIVEY  = 0x00001000
DDSCAPS2_CUBEMAP_NEGATIVEY  = 0x00002000
DDSCAPS2_CUBEMAP_POSITIVEZ  = 0x00004000
DDSCAPS2_CUBEMAP_NEGATIVEZ  = 0x00008000
DDSCAPS2_VOLUME             = 0x00200000

class DdsHeader:
	""" The header of a dds file """

	_fields = [
		('dwMagic', '4s'),
		('dwSize', 'I'),
		('dwFlags', 'I'),
		('dwHeight', 'I'),
		('dwWidth', 'I'),
		('dwPitchOrLinearSize', 'I'),
		('dwDepth', 'I'),
		('dwMipMapCount', 'I'),
		('dwReserved1', '44s'),
	
		# Pixel format
		('dwSize', 'I'),
		('dwFlags', 'I'),
		('dwFourCC', '4s'),
		('dwRGBBitCount', 'I'),
		('dwRBitMask', 'I'),
		('dwGBitMask', 'I'),
		('dwBBitMask', 'I'),
		('dwRGBAlphaBitMask', 'I'),

		('dwCaps1', 'I'),
		('dwCaps2', 'I'),
		('dwCapsReserved', '8s'),
		('dwReserved2', 'I')]

	def __init__(self, buff):
		buff_format = self.get_format()
		items = struct.unpack(buff_format, buff)
		for field, value in map(None, self._fields, items):
			setattr(self, field[0], value)

	@classmethod
	def get_format(cls):
		return '<' + ''.join([f[1] for f in cls._fields])

	@classmethod
	def get_size(cls):
		return struct.calcsize(cls.get_format())

#
# ETC2
#
	
class PkmHeader:
	""" The header of a pkm file """
	
	_fields = [
		("magic", "6s"),
		("type", "H"),
		("width", "H"),
		("height", "H"),
		("origWidth", "H"),
		("origHeight", "H")]

	def __init__(self, buff):
		buff_format = self.get_format()
		items = struct.unpack(buff_format, buff)
		for field, value in map(None, self._fields, items):
			setattr(self, field[0], value)

	@classmethod
	def get_format(cls):
		return ">" + "".join([f[1] for f in cls._fields])

	@classmethod
	def get_size(cls):
		return struct.calcsize(cls.get_format())
	
#
# Functions
# 

def printi(s):
	print("[I] %s" % s)

def printw(s):
	print("[W] %s" % s)

def is_power2(num):
	""" Returns true if a number is a power of two """
	return num != 0 and ((num & (num - 1)) == 0)

def get_base_fname(path):
	""" From path/to/a/file.ext return the "file" """
	return os.path.splitext(os.path.basename(path))[0]

def parse_commandline():
	""" Parse the command line arguments """

	parser = optparse.OptionParser(usage = "usage: %prog [options]", \
			description = "This program converts a single image or a number " \
			"of images (for 3D and 2DArray textures) to AnKi texture format." \
			" It requires 4 different applications/executables to " \
			"operate: convert, identify, nvcompress and etcpack. These " \
			"applications should be in PATH except the convert where you " \
			"need to define the executable explicitly")

	parser.add_option("-i", "--input", dest = "inp",
			type = "string", help = "specify the image(s) to convert. " \
			"Seperate with :")

	parser.add_option("-o", "--output", dest = "out",
			type = "string", help = "specify new image. ")

	parser.add_option("-t", "--type", dest = "type",
			type = "string", default = "2D", 
			help = "type of the image (2D or cube or 3D or 2DArray)")

	parser.add_option("-f", "--fast", dest = "fast",
			action = "store_true", default = False, 
			help = "run the fast version of the converters")

	parser.add_option("-n", "--normal", dest = "normal",
			action = "store_true", default = False, 
			help = "assume the texture is normal")

	parser.add_option("-c", "--convert-path", dest = "convert_path", 
			type = "string", default = "/usr/bin/convert", 
			help = "the executable where convert tool is " \
			"located. Stupid etcpack cannot get it from PATH")

	parser.add_option("--no-alpha", dest = "no_alpha", 
			action = "store_true", default = False, 
			help = "remove alpha channel")

	parser.add_option("--no-uncompressed", dest = "no_uncompressed", 
			action = "store_true", default = True, 
			help = "don't store uncompressed data")

	parser.add_option("--to-linear-rgb", dest = "to_linear_rgb", 
			action = "store_true", default = False, 
			help = "assume the input textures are sRGB. If this option is " \
			"true then convert them to linear RGB")

	# Add the default value on each option when printing help
	for option in parser.option_list:
		if option.default != ("NO", "DEFAULT"):
			option.help += (" " if option.help else "") + "[default: %default]"

	(options, args) = parser.parse_args()

	if not options.inp or not options.out or not options.convert_path:
		parser.error("argument is missing")

	if options.type == "2D":
		typ = TT_2D
	elif options.type == "cube":
		typ = TT_CUBE
	elif options.type == "3D":
		typ = TT_3D
	elif options.type == "2DArray":
		typ = TT_2D_ARRAY
	else:
		parser.error("Unrecognized type: " + options.type)

	config = Config()
	config.in_files = options.inp.split(":")
	config.out_file = options.out
	config.fast = options.fast
	config.type = typ
	config.normal = options.normal
	config.convert_path = options.convert_path
	config.no_alpha = options.no_alpha
	config.no_uncompressed = options.no_uncompressed
	config.to_linear_rgb = options.to_linear_rgb

	return config

def identify_image(in_file):
	""" Return the size of the input image and the internal format """

	color_format = CF_NONE
	width = 0
	height = 0

	proc = subprocess.Popen(["identify", "-verbose" , in_file],
			stdout=subprocess.PIPE)

	stdout_str = proc.stdout.read()

	# Make sure the colorspace is what we want
	"""reg = re.search(r"Colorspace: (.*)", stdout_str)
	if not reg or reg.group(1) != "RGB":
		raise Exception("Something is wrong with the colorspace")"""

	# Get the size of the iamge
	reg = re.search(r"Geometry: ([0-9]*)x([0-9]*)\+", stdout_str)

	if not reg:
		raise Exception("Cannot extract size")
	
	# Identify the color space
	"""if not re.search(r"red: 8-bit", stdout_str) \
			or not re.search(r"green: 8-bit", stdout_str) \
			or not re.search(r"blue: 8-bit", stdout_str): \
		raise Exception("Incorrect channel depths")"""

	if re.search(r"alpha: 8-bit", stdout_str):
		color_format = CF_RGBA8
		color_format_str = "RGBA"
	else:
		color_format = CF_RGB8
		color_format_str = "RGB"

	# print some stuff and return
	printi("width: %s, height: %s color format: %s" % \
			(reg.group(1), reg.group(2), color_format_str))

	return (color_format, int(reg.group(1)), int(reg.group(2)))

def create_mipmaps(in_file, tmp_dir, width_, height_, color_format, \
		to_linear_rgb):
	""" Create a number of images for all mipmaps """

	printi("Generate mipmaps")

	width = width_
	height = height_

	mips_fnames = []

	while width >= 4 and height >= 4:
		size_str = "%dx%d" % (width, height)
		out_file_str = os.path.join(tmp_dir, get_base_fname(in_file)) \
				+ "." + size_str

		printi("  %s.tga" % out_file_str)

		mips_fnames.append(out_file_str)

		args = ["convert", in_file]
		
		# to linear
		if to_linear_rgb:
			if color_format != CF_RGB8:
				raise Exception("to linear RGB only supported for RGB textures")
			
			args.append("-set") 
			args.append("colorspace")
			args.append("sRGB")
			args.append("-colorspace")
			args.append("RGB")

		# resize
		args.append("-resize") 
		args.append(size_str) 

		# alpha
		args.append("-alpha") 
		if color_format == CF_RGB8:
			args.append("deactivate")
		else:
			args.append("activate")

		args.append(out_file_str + ".tga")
		subprocess.check_call(args)

		width = width / 2
		height = height / 2

	return mips_fnames

def create_etc_images(mips_fnames, tmp_dir, fast, color_format, convert_path):
	""" Create the etc files """

	printi("Creating ETC images")

	# Copy the convert tool to the working dir so that etcpack will see it
	shutil.copy2(convert_path, \
			os.path.join(tmp_dir, os.path.basename(convert_path)))

	for fname in mips_fnames:
		# Unfortunately we need to flip the image. Use convert again
		in_fname = fname + ".tga"
		flipped_fname = fname + "_flip.tga"
		args = ["convert", in_fname, "-flip", flipped_fname]
		subprocess.check_call(args)
		in_fname = flipped_fname

		printi("  %s" % in_fname)

		args = ["etcpack", in_fname, tmp_dir, "-c", "etc2"]

		if fast:
			args.append("-s")
			args.append("fast")

		args.append("-f")
		if color_format == CF_RGB8:
			args.append("RGB")
		else:
			args.append("RGBA")

		# Call the executable AND change the working directory so that etcpack
		# will find convert
		subprocess.check_call(args, stdout = subprocess.PIPE, cwd = tmp_dir)

def create_dds_images(mips_fnames, tmp_dir, fast, color_format, normal):
	""" Create the dds files """

	printi("Creating DDS images")

	for fname in mips_fnames:
		# Unfortunately we need to flip the image. Use convert again
		in_fname = fname + ".tga"
		flipped_fname = fname + "_flip.tga"
		args = ["convert", in_fname, "-flip", flipped_fname]
		subprocess.check_call(args)
		in_fname = flipped_fname

		# Continue
		out_fname = os.path.join(tmp_dir, os.path.basename(fname) + ".dds")

		printi("  %s" % out_fname)

		args = ["nvcompress", "-silent", "-nomips"]

		if fast:
			args.append("-fast")

		if color_format == CF_RGB8:
			if not normal:
				args.append("-bc1")
			else:
				args.append("-bc1n")
		elif color_format == CF_RGBA8:
			args.append("-alpha")
			if not normal:
				args.append("-bc3")
			else:
				args.append("-bc3n")

		args.append(in_fname)
		args.append(out_fname)

		subprocess.check_call(args, stdout = subprocess.PIPE)

def write_raw(tex_file, fname, width, height, color_format):
	""" Append raw data to the AnKi texture file """

	printi("  Appending %s" % fname)

	# Read and check the header
	uncompressed_tga_header = struct.pack("BBBBBBBBBBBB", \
			0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0)

	in_file = open(fname, "rb")
	tga_header = in_file.read(12)

	if len(tga_header) != 12:
		raise Exception("Failed reading TGA header")
	
	if uncompressed_tga_header != tga_header:
		raise Exception("Incorrect TGA header")

	# Read the size and bpp
	header6_buff = in_file.read(6)

	if len(header6_buff) != 6:
		raise Exception("Failed reading TGA header #2")
		
	header6 = struct.unpack("BBBBBB", header6_buff)

	img_width = header6[1] * 256 + header6[0]
	img_height = header6[3] * 256 + header6[2]
	img_bpp = header6[4];

	if (color_format != CF_RGB8 or img_bpp != 24) \
			and (color_format != CF_RGBA8 or img_bpp != 32):
		raise Exception("Unexpected bpp")
		
	if img_width != width or img_height != height:
		raise Exception("Unexpected width or height")

	# Read the data
	data_size = width * height
	if color_format == CF_RGB8:
		data_size *= 3
	else:
		data_size *= 4

	data = bytearray(in_file.read(data_size))

	if len(data) != data_size:
		raise Exception("Failed to read all data")

	tmp = in_file.read(128)
	if len(tmp) != 0:
		printw("  File shouldn't contain more data")

	# Swap colors
	bpp = img_bpp / 8
	for i in xrange(0, data_size, bpp):
		temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;

	# Write data to tex_file
	tex_file.write(data)

def write_s3tc(out_file, fname, width, height, color_format):
	""" Append s3tc data to the AnKi texture file """

	# Read header
	printi("  Appending %s" % fname)
	in_file = open(fname, "rb")

	header = in_file.read(DdsHeader.get_size())

	if len(header) != DdsHeader.get_size():
		raise Exception("Failed to read DDS header")

	dds_header = DdsHeader(header)

	if dds_header.dwWidth != width or dds_header.dwHeight != height:
		raise Exception("Incorrect width")

	if color_format == CF_RGB8 \
			and dds_header.dwFourCC != "DXT1":
		raise Exception("Incorrect format. Expecting DXT1")

	if color_format == CF_RGBA8 \
			and dds_header.dwFourCC != "DXT5":
		raise Exception("Incorrect format. Expecting DXT5")

	# Read and write the data
	if color_format == CF_RGB8:
		block_size = 8
	else:
		block_size = 16

	data_size = (width / 4) * (height / 4) * block_size

	data = in_file.read(data_size)

	if len(data) != data_size:
		raise Exception("Failed to read DDS data")

	# Make sure that the file doesn't contain any more data
	tmp = in_file.read(1)
	if len(tmp) != 0:
		printw("  File shouldn't contain more data")

	out_file.write(data)

def write_etc(out_file, fname, width, height, color_format):
	""" Append etc2 data to the AnKi texture file """
	
	printi("  Appending %s" % fname)

	# Read header
	in_file = open(fname, "rb")

	header = in_file.read(PkmHeader.get_size())

	if len(header) != PkmHeader.get_size():
		raise Exception("Failed to read PKM header")

	pkm_header = PkmHeader(header)

	if pkm_header.magic != "PKM 20":
		raise Exception("Incorrect PKM header")

	if width != pkm_header.width or height != pkm_header.height:
		raise Exception("Incorrect PKM width or height")

	# Read and write the data
	data_size = (pkm_header.width / 4) * (pkm_header.height / 4) * 8

	data = in_file.read(data_size)

	if len(data) != data_size:
		raise Exception("Failed to read PKM data")

	# Make sure that the file doesn't contain any more data
	tmp = in_file.read(1)
	if len(tmp) != 0:
		printw("  File shouldn't contain more data")

	out_file.write(data)

def convert(config):
	""" This is the function that does all the work """

	# Invoke app named "identify" to get internal format and width and height
	(color_format, width, height) = identify_image(config.in_files[0])

	if not is_power2(width) or not is_power2(height):
		raise Exception("Image width and height should power of 2")

	if color_format == CF_RGBA8 and config.normal:
		raise Exception("RGBA image and normal does not make much sense")

	for i in range(1, len(config.in_files)):
		(color_format_2, width_2, height_2) = identify_image(in_files[i])

		if width != width_2 or height != height_2 \
				or color_format != color_format_2:
			raise Exception("Images are not same size and color space")

	if config.no_alpha:
		color_format = CF_RGB8

	# Create images
	for in_file in config.in_files:
		mips_fnames = create_mipmaps(in_file, config.tmp_dir, width, height, \
				color_format, config.to_linear_rgb)

		# Create etc images
		create_etc_images(mips_fnames, config.tmp_dir, config.fast, \
				color_format, config.convert_path)

		# Create dds images
		create_dds_images(mips_fnames, config.tmp_dir, config.fast, \
				color_format, config.normal)

	# Open file
	fname = config.out_file
	printi("Writing %s" % fname)
	tex_file = open(fname, "wb")

	# Write header
	ak_format = "8sIIIIIIII"

	data_compression = DC_S3TC | DC_ETC2
	if not config.no_uncompressed:
		data_compression = data_compression | DC_RAW

	buff = struct.pack(ak_format, 
			b"ANKITEX1",
			width,
			height,
			len(config.in_files),
			config.type,
			color_format,
			data_compression,
			config.normal,
			len(mips_fnames))

	tex_file.write(buff)

	# Write header padding
	header_padding_size = 128 - struct.calcsize(ak_format)

	if header_padding_size != 88:
		raise Exception("Check the header")

	for i in range(0, header_padding_size):
		tex_file.write('\0')

	# For each compression
	for compression in range(0, 3):

		tmp_width = width
		tmp_height = height

		# For each level
		while tmp_width >= 4 and tmp_height >= 4:

			# For each image
			for in_file in config.in_files:
				size_str = "%dx%d" % (tmp_width, tmp_height)
				in_base_fname = os.path.join(config.tmp_dir, \
						get_base_fname(in_file)) + "." + size_str

				# Write RAW
				if compression == 0 and not config.no_uncompressed:
					write_raw(tex_file, in_base_fname + ".tga", \
							tmp_width, tmp_height, color_format)
				# Write S3TC
				elif compression == 1:
					write_s3tc(tex_file, in_base_fname + ".dds", \
							tmp_width, tmp_height, color_format)
				# Write ETC
				elif compression == 2:
					write_etc(tex_file, in_base_fname + "_flip.pkm", \
							tmp_width, tmp_height, color_format)
			
			tmp_width = tmp_width / 2
			tmp_height = tmp_height / 2

def main():
	""" The main """

	# Parse cmd line args
	config = parse_commandline();

	if config.type == TT_CUBE and len(config.in_files) != 6:
		raise Exception("Not enough images for cube generation")

	if (config.type == TT_3D or config.type == TT_2D_ARRAY) \
			and len(config.in_files) < 2:
		#raise Exception("Not enough images for 2DArray/3D texture")
		printw("Not enough images for 2DArray/3D texture")

	if config.type == TT_2D and len(config.in_files) != 1:
		raise Exception("Only one image for 2D textures needed")

	if not os.path.isfile(config.convert_path):
		raise Exception("Tool convert not found: " + config.convert_path)

	# Setup the temp dir
	config.tmp_dir = tempfile.mkdtemp("_ankitex")

	# Do the work
	try:
		convert(config)
	finally:
		shutil.rmtree(config.tmp_dir)
		
	# Done
	printi("Done!")

if __name__ == "__main__":
	main()
