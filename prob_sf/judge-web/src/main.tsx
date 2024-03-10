import React from 'react'
import ReactDOM from 'react-dom'
import 'antd/dist/reset.css'
import ExpandedTable from './ExpandedTable';

const App = () => {
  const expanded = true;
  return <ExpandedTable expanded={expanded} fatherFull={expanded} />;
};

ReactDOM.render(
  <React.StrictMode>
    <App />
  </React.StrictMode>,
  document.getElementById('root')
)
