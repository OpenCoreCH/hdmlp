import hdmlp
import time

batch_size = 128
epochs = 10

lib_path = "/Volumes/GoogleDrive/Meine Ablage/Dokumente/1 - Schule/1 - ETHZ/6. Semester/Bachelor Thesis/hdmlp/cpp/hdmlp/cmake-build-debug/libhdmlp.dylib"
config_path = "/Volumes/GoogleDrive/Meine Ablage/Dokumente/1 - Schule/1 - ETHZ/6. Semester/Bachelor Thesis/hdmlp/cpp/hdmlp/data/hdmlp.cfg"
#path = "/tmp/test"
path = "/Volumes/Daten/Daten/Datasets/hymenoptera_data/train" # (245 files)

job = hdmlp.Job(path,
                batch_size,
                epochs,
                "uniform",
                True,
                None,
                config_path,
                lib_path)
job.setup()
#print("Setup complete")

for i in range(batch_size * epochs):
    label, sample = job.get()
    #time.sleep(1)
    #print(label)
    #print(sample)


job.destroy()