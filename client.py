import hdmlp
from PIL import Image
import io

batch_size = 128
epochs = 1

job = hdmlp.Job("/tmp/hymenoptera_data/train",
                batch_size,
                epochs,
                "uniform",
                True,
                None)
job.setup()
print("Setup complete")

for i in range(batch_size * epochs):
    label, sample = job.get()
    img = Image.open(io.BytesIO(sample))
    img.convert('RGB')
    #img.show()


job.destroy()