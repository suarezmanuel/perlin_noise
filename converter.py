import numpy as np
from PIL import Image
import os
import amulet
from amulet.api.block import Block
from amulet.api.chunk import Chunk
from amulet.api.data_types import Dimension

# Load Perlin noise image
image = Image.open('perlin_noise.png').convert('L')
width, height = image.size
data = np.array(image)

# Constants
chunk_size = 16
image_width = 800
chunks_per_side = image_width // chunk_size  # 50 for 800x800 image

# Create a new world directory
world_path = '/Users/manuel/Library/Application Support/minecraft/saves/perlin'
if not os.path.exists(world_path):
    os.makedirs(world_path)

# Load the existing world
world = amulet.load_level(world_path)

# Function to create and save a chunk
def create_chunk(world, chunk_x, chunk_z):
    dimension = Dimension.OVERWORLD  # Using the overworld dimension

    # Check if the chunk already exists
    try:
        chunk = world.get_chunk(chunk_x, chunk_z, dimension)
    except:
        chunk = Chunk(chunk_x, chunk_z)
        chunk.blocks = np.zeros((16, 256, 16), dtype=np.uint32)  # Initialize blocks array for chunk

    # Access the chunk palette
    palette = chunk.block_palette

    for x in range(chunk_size):
        for z in range(chunk_size):
            global_x = chunk_x * chunk_size + x
            global_z = chunk_z * chunk_size + z
            height = data[global_z, global_x] // 4  # Scale down height

            for y in range(height):
                block = Block("minecraft", "stone")
                block_id = palette.get_add_block(block)
                chunk.blocks[x, y, z] = block_id

            block = Block("minecraft", "grass_block")
            block_id = palette.get_add_block(block)
            chunk.blocks[x, height, z] = block_id

    world.add_chunk(chunk, dimension)

# Create chunks
for chunk_x in range(chunks_per_side):
    for chunk_z in range(chunks_per_side):
        create_chunk(world, chunk_x, chunk_z)

# Save the world
world.save()

print("World created and saved successfully.")
