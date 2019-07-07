from dataset.coqa import CoQAReader,CoQAEvaluator
from libraries.BertWrapper import BertDataHelper
from model.bert_coqa import BertCoQA
from data.vocabulary import  Vocabulary
import logging
import sys
import os

os.environ['CUDA_VISIBLE_DEVICES'] = '7'
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')

coqa_reader = CoQAReader(-1)
data_folder='./data/'
train_filename = "coqa-train-v1.0.json"
eval_filename = 'coqa-dev-v1.0.json'
vocab = Vocabulary(do_lowercase=True)
train_data = coqa_reader.read(data_folder+train_filename, 'train')
eval_data = coqa_reader.read(data_folder+eval_filename,'dev')
vocab.build_vocab(train_data+eval_data)
vocab_save_path='save_model/vocab.json'
vocab.save(vocab_save_path)
evaluator = CoQAEvaluator(data_folder+eval_filename)
bert_dir = './'
bert_data_helper = BertDataHelper(bert_dir)
train_data = bert_data_helper.convert(train_data,data='coqa')
eval_data = bert_data_helper.convert(eval_data,data='coqa')



from sogou_mrc.data.batch_generator import BatchGenerator
train_batch_generator = BatchGenerator(vocab,train_data,training=True,batch_size=4,additional_fields=[
    'input_ids','segment_ids','input_mask','start_position','end_position',
    'question_mask','rationale_mask','yes_mask','extractive_mask','no_mask','unk_mask','qid'
])
eval_batch_generator = BatchGenerator(vocab,eval_data,training=False,batch_size=4,additional_fields=['input_ids','segment_ids','input_mask','start_position','end_position',
    'question_mask','rationale_mask','yes_mask','extractive_mask','no_mask','unk_mask','qid'])

model = BertCoQA(bert_dir=bert_dir,answer_verification=True)
warmup_proportion = 0.1
num_train_steps = int(
        len(train_data) / 12 * 2)
num_warmup_steps = int(num_train_steps * warmup_proportion)

# original paper adamax optimizer
model.compile(3e-5,num_train_steps=num_train_steps,num_warmup_steps=num_warmup_steps)
model.train_and_evaluate(train_batch_generator, eval_batch_generator,evaluator, epochs=2, eposides=1,save_dir='./save_model2/')
#model.evaluate(eval_batch_generator,evaluator)

